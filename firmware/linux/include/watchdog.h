/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2024
 *
 */

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <linux/mutex.h>       // For struct mutex
#include <linux/sched.h>       // For struct task_struct

typedef struct
{
    char indicator;
    struct task_struct *threadHandle;
    struct mutex watchdogMutex;
} watchdogProcess;

watchdogProcess* watchdog_getProcess(void);


#endif // WATCHDOG_H
