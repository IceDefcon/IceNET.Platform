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
    IDLE    = 0,
    SPI     = 1,
    I2C     = 2,    
    USER    = 3
} stateMachineType;

/* GET STATE */ struct stateMachineStaus* get_stateMachineStaus(void);
/* SET STATE */ void set_stateMachineState(stateMachineType newState);


void stateMachineInit(void);
void stateMachineDestroy(void);

#endif // STATE_MACHINE_H