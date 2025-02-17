/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <linux/mutex.h>       // For struct mutex
#include <linux/sched.h>       // For struct task_struct

typedef struct
{
    bool indicatorFPGA;
    char indicatorCurrent;
    char indicatorPrevious;
    struct task_struct *threadHandle;
    struct mutex watchdogMutex;
} watchdogProcess;

void watchdog_lockWatchdogMutex(void);
void watchdog_unlockWatchdogMutex(void);
watchdogProcess* watchdog_getProcess(void);

void watchdogInit(void);
void watchdogDestroy(void);

/* GET */ bool getIndicatorFPGA(void);

#endif // WATCHDOG_H
