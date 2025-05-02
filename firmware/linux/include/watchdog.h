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

#include "types.h"

typedef struct
{
    bool indicatorFPGA;
    char indicatorCurrent;
    char indicatorPrevious;
    struct task_struct *threadHandle;
    spinlock_t watchdogSpinlock;
    unsigned long irqflags;
    const char threadName[];
} watchdogProcess;

void watchdog_spinLockCtrl(CtrlType ctrl);
watchdogProcess* watchdog_getProcess(void);

void watchdogInit(void);
void watchdogDestroy(void);

/* GET */ bool getIndicatorFPGA(void);

#endif // WATCHDOG_H
