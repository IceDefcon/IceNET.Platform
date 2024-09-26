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
#include "watchdog.h"

static watchdogProcess Process =
{
    .indicator = 0x00,
    .threadHandle = NULL,
    .watchdogMutex = __MUTEX_INITIALIZER(Process.watchdogMutex),
};

watchdogProcess* watchdog_getProcess(void)
{
	return &Process;
}
