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
    IDLE = 0,
    DMA,
    SPI,
    INTERRUPT,
    KILL
} stateType;

typedef struct
{
    stateType currentState;
    struct task_struct *threadHandle;
    struct mutex stateMutex;
    bool dmaStop;
} stateMachineProcess;

/* SET */ void setStateMachine(stateType newState);
/* GET */ stateType getStateMachine(void);

void stateMachineInit(void);
void stateMachineDestroy(void);

#endif // STATE_MACHINE_H
