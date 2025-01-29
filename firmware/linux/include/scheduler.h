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
    SCH_INIT = 0,
    SCH_CONFIG,
    SCH_IDLE,
    SCH_MAIN_20MS,
    SCH_AMOUNT,
} schedulerType;

typedef struct
{
    schedulerType currentState;
    struct task_struct *threadHandle;
    struct mutex stateMutex;
    bool configDone;
} schedulerProcess;

/* SET */ void setScheduler(schedulerType newState);
/* GET */ schedulerType getScheduler(void);
/* READY */ bool isShedulerReady(void);

void schedulerInit(void);
void schedulerDestroy(void);

#endif // SCHEDULER_H
