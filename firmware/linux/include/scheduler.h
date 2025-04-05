/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#define ALLOCATION_DELAY            (300) /* Allocation Delay in Seconds */
#define SCHEDULER_FPGA_PERIOD       (20) /* Pulse From PPGA every 20ms */
#define SECOND_SCHEDULER_TICKS      (1000/SCHEDULER_FPGA_PERIOD) /* 1s = 20ms * 50 tics */
#define ALLOCATION_DELAY_TICKS(x)   (x * SECOND_SCHEDULER_TICKS) /* 300s = 20ms * 15000 ticks */
#define ALLOCATION_PRINT_DELAY      (ALLOCATION_DELAY_TICKS(ALLOCATION_DELAY)) /* 50 * 300 = 15000 Scheduler 20ms ticks -> 300s */

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
    struct mutex stateMutex;
    bool configDone;
    uint32_t allocationTimer;
} schedulerProcess;

/* RUN */ void schedulerTimerRun(struct work_struct *work);
/* SET */ void setScheduler(schedulerType newState);
/* GET */ schedulerType getScheduler(void);
/* READY */ bool isShedulerReady(void);

void schedulerInit(void);
void schedulerDestroy(void);

#endif // SCHEDULER_H
