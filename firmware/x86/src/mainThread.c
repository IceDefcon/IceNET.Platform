/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/delay.h> // For msleep
#include <linux/gpio.h>

#include "networkControl.h"
#include "transmitter.h"
#include "mainThread.h"
#include "receiver.h"

/////////////////////////
//                     //
//                     //
//                     //
// [STM] State Machine //
//                     //
//                     //
//                     //
/////////////////////////

static mainThreadProcess Process =
{
    .currentState = MAIN_THREAD_IDLE,
    .previousState = MAIN_THREAD_IDLE,
    .threadHandle = NULL,
    .irqFlags = 0,
};

/* SET */ void setStateMachine(mainThreadStateType newState)
{
    spin_lock_irqsave(&Process.smSpinlock, Process.irqFlags);
    Process.currentState = newState;
    spin_unlock_irqrestore(&Process.smSpinlock, Process.irqFlags);
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
        "MAIN_THREAD_NETWORK_ARP_REQUEST",
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
        return "UNKNOWN_MAIN_THRAD";
    }
}

static int mainThread(void *data)
{
    mainThreadStateType state;

    while (!kthread_should_stop())
    {
        state = getStateMachine();

        if(Process.previousState != Process.currentState)
        {
            printk(KERN_INFO "[CTRL][STM] mainThread State Machine %d->%d %s\n",Process.previousState, Process.currentState, getMainThreadStateString(Process.currentState));
            Process.previousState = Process.currentState;
        }

        switch(state)
        {
            case MAIN_THREAD_IDLE:
                /* Nothing here :: Just wait */
                break;

            case MAIN_THREAD_NETWORK_ARP_REQUEST:
                msleep(1000); /* Debug Remive me later */
                printk(KERN_INFO "[CTRL][STM] mode -> MAIN_THREAD_NETWORK_ARP_REQUEST\n");
                arpSendRequest();
                setStateMachine(MAIN_THREAD_TCP_TRANSMISSION);
                break;

            case MAIN_THREAD_TCP_TRANSMISSION:
                printk(KERN_INFO "[CTRL][STM] mode -> MAIN_THREAD_TCP_TRANSMISSION\n");
                tcpTransmission();
                setStateMachine(MAIN_THREAD_UDP_TRANSMISSION);
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
                return -EINVAL; // Proper error code
        }

        /**
         *
         * Reduce consumption of CPU resources
         * Add a short delay to prevent
         * busy waiting
         *
         */
        msleep(10); /* Release 90% of CPU resources */

    }

    return 0;
}

void mainThreadInit(void)
{
    spin_lock_init(&Process.smSpinlock);
    setStateMachine(MAIN_THREAD_NETWORK_ARP_REQUEST);

    Process.threadHandle = kthread_create(mainThread, NULL, "iceMainThread");

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
