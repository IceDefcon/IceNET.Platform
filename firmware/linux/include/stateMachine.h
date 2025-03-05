/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

///////////////////
//               //
//               //
//               //
// State Machine //
//               //
//               //
//               //
///////////////////

typedef enum 
{
    SM_IDLE = 0,
    SM_LONG_DMA,
    SM_DMA,
    SM_SPI,
    SM_OFFLOAD,
    SM_PRINT,
    SM_CMD,
    SM_DONE,
} stateMachineType;

typedef struct
{
    stateMachineType currentState;
    struct task_struct *threadHandle;
    spinlock_t smSpinlock;
    unsigned long irqFlags;
} stateMachineProcess;

/* SET */ void setStateMachine(stateMachineType newState);
/* GET */ stateMachineType getStateMachine(void);

void stateMachineInit(void);
void stateMachineDestroy(void);

#endif // STATE_MACHINE_H
