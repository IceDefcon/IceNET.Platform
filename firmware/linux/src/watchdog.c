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
#include "watchdog.h"
#include "console.h"

static watchdogProcess Process =
{
    .initFlag = false,
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
    int len;
    char message[128];

    while (!kthread_should_stop())
    {
		mutex_lock(&Process.watchdogMutex);
        memset(message, 0, sizeof(message));
        if(Process.indicatorPrevious != Process.indicatorCurrent)
        {
            printk(KERN_INFO "[CTRL][WDG] Watchdog Live [%x|%x]\n", Process.indicatorPrevious, Process.indicatorCurrent);
            len = snprintf(message, sizeof(message), "[CTRL][WDG] Watchdog Live [%x|%x]\n", Process.indicatorPrevious, Process.indicatorCurrent);
        }
        else
        {
            /**
             * Current design make 1st interation to take 5s
             * So 1s watchdog should update indicator on next interration
             * Giving valid information about Live/Dead Watchdog signal
             **/
            if(Process.initFlag == true)
            {
                /* This message dont have to be send over UART since FPGA is Dead and not looping trough UART signal to FTDI */
                printk(KERN_INFO "[CTRL][WDG] Watchdog Dead [%x|%x] ERROR: Please check if FPGA binary is loaded\n", Process.indicatorPrevious, Process.indicatorCurrent);
            }
            else
            {
                Process.initFlag = true;
            }
        }
        uart_write(message, len);
   		Process.indicatorPrevious = Process.indicatorCurrent;
		mutex_unlock(&Process.watchdogMutex);

        /**
         *
         * Reduce consumption of CPU resources
         * Add a short delay to prevent
         * busy waiting
         *
         */
        msleep(5000); /* Release 90% of CPU resources */

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
