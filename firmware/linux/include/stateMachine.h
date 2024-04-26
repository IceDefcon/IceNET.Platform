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

static struct task_struct *thread_handle;

typedef enum 
{
    IDLE    = 0,
    SPI     = 1,
    I2C     = 2,    
    USER    = 3
} stateType;

static struct stateMachine 
{ 
    stateType state;
} stateStaus;

/* GET STATE */ struct stateMachine* get_stateMachine(void);
/* SET STATE */ void set_stateMachine(stateType newState);


void stateMachineInit(void);
void stateMachineDestroy(void);

#endif // STATE_MACHINE_H