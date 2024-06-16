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

#define SM_OK 0

typedef enum 
{
    IDLE = 0,
    SPI,
    INTERRUPT,
    FEEDBACK,
    KILL_APPLICATION
} stateType;

/* SET STATE */ void setStateMachine(stateType newState);

void stateMachineInit(void);
void stateMachineDestroy(void);

#endif // STATE_MACHINE_H