#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/wait.h>

#include "transmitter.h"
#include "diagnostics.h"
#include "mainThread.h"
#include "x86network.h"
#include "receiver.h"
#include "memory.h"

/////////////////////////
//                     //
//                     //
//                     //
// [STM] State Machine //
//                     //
//                     //
//                     //
/////////////////////////

static DECLARE_WAIT_QUEUE_HEAD(mainThreadWaitQueue);

static mainThreadProcess Process =
{
    .currentState = MAIN_THREAD_IDLE,
    .previousState = MAIN_THREAD_IDLE,
    .stateChanged = false,
    .threadHandle = NULL,
    .irqFlags = 0,
    .threadName = "iceMainThread",
};

/* SET */ void setStateMachine(mainThreadStateType newState)
{
    spin_lock_irqsave(&Process.smSpinlock, Process.irqFlags);
    Process.currentState = newState;
    Process.stateChanged = true; /* Signal that something changed */
    spin_unlock_irqrestore(&Process.smSpinlock, Process.irqFlags);
    wake_up_interruptible(&mainThreadWaitQueue); /* Wake up thread */
}

/* GET */ mainThreadStateType getStateMachine(void)
{
    mainThreadStateType state;
    spin_lock_irqsave(&Process.smSpinlock, Process.irqFlags);
    state = Process.currentState;
    spin_unlock_irqrestore(&Process.smSpinlock, Process.irqFlags);
    return state;
}

static const char* getMainThreadStateString(mainThreadStateType type)
{
    static const char* mainThreadStateStrings[] =
    {
        "MAIN_THREAD_IDLE",
        "MAIN_THREAD_NETWORK_ICMP_PING",
        "MAIN_THREAD_NETWORK_ARP_REQUEST",
        "MAIN_THREAD_NETWORK_NDP_REQUEST",
        "MAIN_THREAD_TCP_TRANSMISSION",
        "MAIN_THREAD_UDP_TRANSMISSION",
        "MAIN_THREAD_DONE"
    };

    if (type >= 0 && type < MAIN_THREAD_AMOUNT)
    {
        return mainThreadStateStrings[type];
    }
    else
    {
        return "UNKNOWN_MAIN_THREAD";
    }
}

static int mainThread(void *data)
{
    mainThreadStateType state;
    showThreadDiagnostics(Process.threadName);

    while (!kthread_should_stop())
    {
        /* Sleep until stateChanged or stop signal */
        wait_event_interruptible(mainThreadWaitQueue, Process.stateChanged || kthread_should_stop());

        if (kthread_should_stop())
        {
            break;
        }

        /* Clear flag after wake */
        spin_lock_irqsave(&Process.smSpinlock, Process.irqFlags);
        Process.stateChanged = false;
        spin_unlock_irqrestore(&Process.smSpinlock, Process.irqFlags);

        state = getStateMachine();

        if (Process.previousState != Process.currentState)
        {
            printk(KERN_INFO "[CTRL][STM] mainThread State Machine %d->%d %s\n", Process.previousState, Process.currentState, getMainThreadStateString(Process.currentState));
            Process.previousState = Process.currentState;
        }

        switch (state)
        {
            case MAIN_THREAD_IDLE:
                /* Nothing to do */
                break;

            case MAIN_THREAD_NETWORK_ICMP_PING:
                printk(KERN_INFO "[CTRL][STM] mode -> MAIN_THREAD_NETWORK_ICMP_PING\n");
                char ip_str[16];
                __be32 ip;

                /**
                 * TODO :: Do we need configurable Network Mask
                 *
                 * Ping current network mask -> IceNET :: 192.168.8.0/24
                 * /24 -> 11111111 11111111 11111111 00000000
                 * First 24-Bits are not modifiable
                 * do we have range 0 to 255
                 * for the last 8-Bits
                 *
                 */
                for (int i = 1; i < 255; i++)
                {
                    snprintf(ip_str, sizeof(ip_str), "192.168.8.%d", i);
                    ip = in_aton(ip_str);
                    icmpSendPing(ip);
                }
                msleep(1000); /* Wait 1s max for the list update */
                printActiveHosts();
                setStateMachine(MAIN_THREAD_DONE);
                break;

            case MAIN_THREAD_NETWORK_ARP_REQUEST:
                printk(KERN_INFO "[CTRL][STM] mode -> MAIN_THREAD_NETWORK_ARP_REQUEST\n");
                arpSendRequest();
                setStateMachine(MAIN_THREAD_DONE);
                break;

            case MAIN_THREAD_NETWORK_NDP_REQUEST:
                printk(KERN_INFO "[CTRL][STM] mode -> MAIN_THREAD_NETWORK_NDP_REQUEST\n");
                ndpSendRequest();
                setStateMachine(MAIN_THREAD_DONE);
                break;

            case MAIN_THREAD_TCP_TRANSMISSION:
                printk(KERN_INFO "[CTRL][STM] mode -> MAIN_THREAD_TCP_TRANSMISSION\n");
                tcpTransmission();
                setStateMachine(MAIN_THREAD_DONE);
                break;

            case MAIN_THREAD_UDP_TRANSMISSION:
                printk(KERN_INFO "[CTRL][STM] mode -> MAIN_THREAD_UDP_TRANSMISSION\n");
                udpTransmission();
                setStateMachine(MAIN_THREAD_DONE);
                break;

            case MAIN_THREAD_DONE:
                printk(KERN_INFO "[CTRL][STM] mode -> MAIN_THREAD_DONE\n");
                setStateMachine(MAIN_THREAD_IDLE);
                break;

            default:
                printk(KERN_ERR "[CTRL][STM] mode -> Unknown\n");
                return -EINVAL;
        }
    }

    return 0;
}

void mainThreadInit(void)
{
    spin_lock_init(&Process.smSpinlock);
    setStateMachine(MAIN_THREAD_IDLE);

    Process.threadHandle = kthread_create(mainThread, NULL, Process.threadName);

    if (IS_ERR(Process.threadHandle))
    {
        printk(KERN_ERR "[INIT][STM] Failed to create kernel thread. Error code: %ld\n", PTR_ERR(Process.threadHandle));
    }
    else
    {
        printk(KERN_INFO "[INIT][STM] Created kthread for mainThread\n");
        wake_up_process(Process.threadHandle);
    }
}

void mainThreadDestroy(void)
{
    if (Process.threadHandle)
    {
        kthread_stop(Process.threadHandle);
        Process.threadHandle = NULL;
    }
    printk(KERN_INFO "[DESTROY][STM] Destroy State Machine kthread\n");
}
