/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef enum
{
    SCH_IDLE = 0,
    SCH_CONFIG,
    SCH_INIT,
    SCH_MAIN,
    SCH_AMOUNT,
} schedulerType;

typedef struct
{
    schedulerType currentState;
    struct task_struct *threadHandle;
    struct mutex stateMutex;
} schedulerProcess;

/* SET */ void setScheduler(schedulerType newState);
/* GET */ schedulerType getScheduler(void);

void schedulerInit(void);
void schedulerDestroy(void);

#endif // SCHEDULER_H
