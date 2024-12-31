/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
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
    SM_DMA,
    SM_SPI,
    SM_INTERRUPT,
    SM_KILL,
    SM_PRINT,
    SM_CMD,
} stateMachineType;

typedef struct
{
    stateMachineType currentState;
    struct task_struct *threadHandle;
    struct mutex stateMutex;
    bool dmaStop;
} stateMachineProcess;

/* SET */ void setStateMachine(stateMachineType newState);
/* GET */ stateMachineType getStateMachine(void);

void stateMachineInit(void);
void stateMachineDestroy(void);

#endif // STATE_MACHINE_H
