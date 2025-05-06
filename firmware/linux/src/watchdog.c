/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#include <linux/module.h>      // For module macros
#include <linux/kernel.h>      // For kernel functions and logging
#include <linux/sched.h>       // For struct task_struct
#include <linux/spinlock.h>    // For spinlocks
#include <linux/kthread.h>     // For kthread functions
#include <linux/delay.h>       // For msleep

#include "charDevice.h"
#include "watchdog.h"
#include "console.h"
#include "memory.h"
#include "types.h"

static watchdogProcess Process =
{
    .indicatorFPGA = false,
    .indicatorCurrent = 0x00,
    .indicatorPrevious = 0x00,
    .threadHandle = NULL,
    .irqflags = 0,
    .threadName = "iceWatchdog",
};

watchdogProcess* watchdog_getProcess(void)
{
	return &Process;
}

void watchdog_spinLockCtrl(CtrlType ctrl)
{
    if(CTRL_LOCK == ctrl)
    {
        spin_lock_irqsave(&Process.watchdogSpinlock, Process.irqflags);
    }
    else if(CTRL_UNLOCK == ctrl)
    {
        spin_unlock_irqrestore(&Process.watchdogSpinlock, Process.irqflags);
    }
}

/* Kernel state machine */
static int watchdogThread(void *data)
{
    DmaTransferType* watchdogData;
    msleep(500); /* Wait for the watchdog capture from FPGA */
    printk(KERN_INFO "[INIT][WDG] Diagnostics\n");
    showThreadDiagnostics(Process.threadName);

    while (!kthread_should_stop())
    {
        watchdog_spinLockCtrl(CTRL_LOCK);

#if 0 /* Hack :: If Fpga is not flashed */
        Process.indicatorCurrent++;
#endif
        watchdogData = getCharDeviceTransfer(DEVICE_WATCHDOG);
        watchdogData->TxData[0] = Process.indicatorPrevious;
        watchdogData->TxData[1] = Process.indicatorCurrent;
        // charDeviceLockCtrl(DEVICE_WATCHDOG, CTRL_UNLOCK);
        eventWakeUpDevice(DEVICE_WATCHDOG);

        if(Process.indicatorPrevious != Process.indicatorCurrent)
        {
            if(Process.indicatorFPGA == false)
            {
                /* This is here :: In case if we have a power-cut in FPGA */
                if(Process.indicatorCurrent - Process.indicatorPrevious != 1)
                {
                    printk(KERN_INFO "[CTRL][WDG] Watchdog Live [%x|%x] Start kicking the Thread\n", Process.indicatorPrevious, Process.indicatorCurrent);
                    Process.indicatorFPGA = true;
                }
            }
        }
        else
        {
            printk(KERN_INFO "[CTRL][WDG] Watchdog Dead [%x|%x] ERROR: Please check if FPGA binary is loaded\n", Process.indicatorPrevious, Process.indicatorCurrent);
            // charDeviceLockCtrl(DEVICE_COMMANDER, CTRL_UNLOCK);
            eventWakeUpDevice(DEVICE_COMMANDER);
            Process.indicatorFPGA = false;
        }

        /* Update indicator and unlock Watchdog Spinlock */
        Process.indicatorPrevious = Process.indicatorCurrent;
        watchdog_spinLockCtrl(CTRL_UNLOCK);

        /**
         *
         * Long delays like 1000ms
         * still prevents thread
         * busy waiting
         *
         */
        msleep(1000); /* Release 90% of CPU resources */

    }

    return 0;
}

void watchdogInit(void)
{
    spin_lock_init(&Process.watchdogSpinlock);

    Process.threadHandle = kthread_create(watchdogThread, NULL, Process.threadName);

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

bool getIndicatorFPGA(void)
{
    return Process.indicatorFPGA;
}
