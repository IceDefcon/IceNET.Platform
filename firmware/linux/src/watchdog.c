/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2024
 *
 */

#include <linux/module.h>      // For module macros
#include <linux/kernel.h>      // For kernel functions and logging
#include <linux/mutex.h>       // For mutex operations
#include <linux/sched.h>       // For struct task_struct
#include <linux/kthread.h>      // For kthread functions
#include <linux/delay.h>        // For msleep
#include "charDevice.h"
#include "watchdog.h"
#include "console.h"
#include "types.h"

static watchdogProcess Process =
{
    .indicatorFPGA = false,
    .indicatorCurrent = 0x00,
    .indicatorPrevious = 0x00,
    .threadHandle = NULL,
    .watchdogMutex = __MUTEX_INITIALIZER(Process.watchdogMutex),
};

watchdogProcess* watchdog_getProcess(void)
{
	return &Process;
}

void watchdog_lockWatchdogMutex(void)
{
	mutex_lock(&Process.watchdogMutex);
}

void watchdog_unlockWatchdogMutex(void)
{
	mutex_unlock(&Process.watchdogMutex);
}

/* Kernel state machine */
static int watchdogThread(void *data)
{
    DataTransfer* watchdogData;
    msleep(500); /* Wait for the watchdog capture from FPGA */

    while (!kthread_should_stop())
    {
		mutex_lock(&Process.watchdogMutex);

        watchdogData = getWatchdogTransfer();
        watchdogData->TxData[0] = Process.indicatorPrevious;
        watchdogData->TxData[1] = Process.indicatorCurrent;
        watchdogData->length = 2;
        unlockWatchdogMutex();
        if(Process.indicatorPrevious == Process.indicatorCurrent)
        {
            unlockWaitMutex();
        }

        if(Process.indicatorPrevious != Process.indicatorCurrent)
        {
            if(Process.indicatorFPGA == false)
            {
                printk(KERN_INFO "[CTRL][WDG] Watchdog Live [%x|%x]\n", Process.indicatorPrevious, Process.indicatorCurrent);
                Process.indicatorFPGA = true;
            }
        }
        else
        {
            printk(KERN_INFO "[CTRL][WDG] Watchdog Dead [%x|%x] ERROR: Please check if FPGA binary is loaded\n", Process.indicatorPrevious, Process.indicatorCurrent);
            Process.indicatorFPGA = false;
        }

        /* Update indicator and unlock Watchdog Mutex */
   		Process.indicatorPrevious = Process.indicatorCurrent;
		mutex_unlock(&Process.watchdogMutex);

        /**
         *
         * Reduce consumption of CPU resources
         * Add a short delay to prevent
         * busy waiting
         *
         */
        msleep(500); /* Release 90% of CPU resources */

    }

    return 0;
}

void watchdogInit(void)
{
    Process.threadHandle = kthread_create(watchdogThread, NULL, "iceWatchdog");

    if (IS_ERR(Process.threadHandle))
    {
        printk(KERN_ERR "[INIT][WDG] Failed to create kernel thread. Error code: %ld\n", PTR_ERR(Process.threadHandle));
    }
    else
    {
        printk(KERN_INFO "[INIT][WDG] Created kthread for watchdogThread\n");
        wake_up_process(Process.threadHandle);
    }
}

void watchdogDestroy(void)
{
    if (Process.threadHandle)
    {
        kthread_stop(Process.threadHandle);
        Process.threadHandle = NULL;
    }
    printk(KERN_INFO "[DESTROY][WDG] Destroy watchdog kthread\n");
}
