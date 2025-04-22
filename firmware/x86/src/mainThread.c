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
#include "mainThread.h"

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
    .currentState = SM_IDLE,
    .previousState = SM_IDLE,
    .threadHandle = NULL,
    .irqFlags = 0,
};

/* SET */ void setStateMachine(stateMachineType newState)
{
    spin_lock_irqsave(&Process.smSpinlock, Process.irqFlags);
    Process.currentState = newState;
    spin_unlock_irqrestore(&Process.smSpinlock, Process.irqFlags);
}

/* GET */ stateMachineType getStateMachine(void)
{
    stateMachineType state;
    spin_lock_irqsave(&Process.smSpinlock, Process.irqFlags);
    state = Process.currentState;
    spin_unlock_irqrestore(&Process.smSpinlock, Process.irqFlags);
    return state;
}

/* Kernel state machine */
static int mainThread(void *data)
{
    stateMachineType state;

    while (!kthread_should_stop())
    {
        state = getStateMachine();

        switch(state)
        {
            case SM_IDLE:
                /* Nothing here :: Just wait */
                break;

            case SM_NETWORK_INIT:
                printk(KERN_INFO "[CTRL][STM] mode -> SM_NETWORK_INIT\n");
                setStateMachine(SM_DONE);
                break;

            case SM_NETWORK_TCP:
                printk(KERN_INFO "[CTRL][STM] mode -> SM_NETWORK_TCP\n");
                setStateMachine(SM_DONE);
                break;

            case SM_NETWORK_UDP:
                printk(KERN_INFO "[CTRL][STM] mode -> SM_NETWORK_UDP\n");
                setStateMachine(SM_DONE);
                break;

            case SM_NETWORK_REGISTER_ARP:
                printk(KERN_INFO "[CTRL][STM] mode -> SM_NETWORK_REGISTER_ARP\n");
                setStateMachine(SM_DONE);
                break;

            case SM_NETWORK_REQUEST_ARP:
                printk(KERN_INFO "[CTRL][STM] mode -> SM_NETWORK_REQUEST_ARP\n");
                setStateMachine(SM_DONE);
                break;

            case SM_DONE:
                printk(KERN_INFO "[CTRL][STM] mode -> SM_DONE\n");
                setStateMachine(SM_IDLE);
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
    setStateMachine(SM_IDLE);

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
