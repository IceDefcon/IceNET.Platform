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
    SPI,
    INTERRUPT,
    KILL_APPLICATION
} stateType;

typedef struct
{
    stateType currentState;
    struct task_struct *thread_handle;
    struct mutex state_mutex;
} stateMachineProcess;

/* SET */ void setStateMachine(stateType newState);
/* GET */ stateType getStateMachine(void);

void stateMachineInit(void);
void stateMachineDestroy(void);

#endif // STATE_MACHINE_H
