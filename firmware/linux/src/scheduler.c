/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h> // For msleep
#include <linux/gpio.h>
#include <linux/mutex.h>

#include "scheduler.h"

////////////////////////
//                    //
//                    //
//                    //
//  [SCH]  Scheduler  //
//                    //
//                    //
//                    //
////////////////////////

static schedulerProcess Process =
{
    .currentState = SCH_IDLE,
    .threadHandle = NULL,
    .stateMutex = __MUTEX_INITIALIZER(Process.stateMutex),
    .configDone = false,
};

/* SET */ void setScheduler(schedulerType newState)
{
    mutex_lock(&Process.stateMutex);
    Process.currentState = newState;
    mutex_unlock(&Process.stateMutex);
}

/* GET */ schedulerType getScheduler(void)
{
    schedulerType state;
    mutex_lock(&Process.stateMutex);
    state = Process.currentState;
    mutex_unlock(&Process.stateMutex);
    return state;
}

/* READY */ bool isShedulerReady(void)
{
    bool ret = false;

    if(true == Process.configDone && SCH_IDLE == getScheduler())
    {
        ret = true;
    }

    return ret;
}

/* Kernel state machine */
static int schedulerThread(void *data)
{
    schedulerType state;

    while (!kthread_should_stop())
    {
        state = getScheduler();

        switch(state)
        {
            case SCH_IDLE:
                // printk(KERN_INFO "[CTRL][SCH] :: SCH_IDLE\n");
                /* Nothing here :: Just wait for state change */
                break;

            case SCH_INIT:
                msleep(100); /* Delay for __init to print everything */
                printk(KERN_INFO "[CTRL][SCH] SCH_INIT\n");
                //
                // TODO
                //
                setScheduler(SCH_CONFIG);
                break;

            case SCH_CONFIG:
                printk(KERN_INFO "[CTRL][SCH] SCH_CONFIG\n");
                //
                // TODO
                //
                Process.configDone = true;
                setScheduler(SCH_MAIN_20MS);
                break;

            case SCH_MAIN_20MS:
                printk(KERN_INFO "[CTRL][SCH] SCH_MAIN_20MS\n");
                /**
                 *
                 * TODO
                 *
                 * Main 10ms loop and
                 * go back to SCH_IDLE state
                 * wait for another 10ms interrupt
                 *
                 */
                setScheduler(SCH_IDLE);
                break;

            default:
                printk(KERN_ERR "[CTRL][SCH] Unknown mode\n");
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

void schedulerInit(void)
{

    Process.threadHandle = kthread_create(schedulerThread, NULL, "iceScehduler");

    if (IS_ERR(Process.threadHandle))
    {
        printk(KERN_ERR "[INIT][SCH] Failed to create kernel thread. Error code: %ld\n", PTR_ERR(Process.threadHandle));
    }
    else
    {
        printk(KERN_INFO "[INIT][SCH] Created kthread for schedulerThread\n");
        wake_up_process(Process.threadHandle);
    }

    setScheduler(SCH_INIT);
}

void schedulerDestroy(void)
{
    if (Process.threadHandle)
    {
        kthread_stop(Process.threadHandle);
        Process.threadHandle = NULL;
    }
    printk(KERN_INFO "[DESTROY][SCH] Destroy Scheduler kthread\n");
}
