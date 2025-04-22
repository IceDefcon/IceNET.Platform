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
#include "memory.h"

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
    .stateMutex = __MUTEX_INITIALIZER(Process.stateMutex),
    .configDone = false,
    .allocationTimer = 0,
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

/* RUN */ void schedulerTimerRun(struct work_struct *work)
{
    schedulerType state;
    state = getScheduler();

    switch(state)
    {
        case SCH_IDLE:
            printk(KERN_INFO "[CTRL][SCH] SCH_IDLE\n");
            /* Nothing here :: Just wait for state change */
            break;

        case SCH_INIT:
            msleep(100); /* Delay for __init to print everything */
            printk(KERN_INFO "[CTRL][SCH] SCH_INIT\n");
            setScheduler(SCH_CONFIG);
            break;

        case SCH_CONFIG:
            printk(KERN_INFO "[CTRL][SCH] SCH_CONFIG\n");
            Process.configDone = true;
            setScheduler(SCH_MAIN_20MS);
            break;

        case SCH_MAIN_20MS:
            /* Chacking currently allocated resources */
            Process.allocationTimer++;
            if(ALLOCATION_PRINT_DELAY == Process.allocationTimer)
            {
                showAllocation();
                Process.allocationTimer = 0;
            }
            setScheduler(SCH_IDLE);
            break;

        default:
            printk(KERN_ERR "[CTRL][SCH] Unknown mode\n");
    }
}

void schedulerInit(void)
{
    Process.allocationTimer = 0;
    setScheduler(SCH_INIT);
}

void schedulerDestroy(void)
{
    printk(KERN_INFO "[DESTROY][SCH] Destroy Scheduler kthread\n");
}
