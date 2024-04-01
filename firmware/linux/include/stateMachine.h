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

#define RETURN_OK 0

enum StateMachine 
{
    IDLE    = 0,
    SPI     = 1,
    I2C     = 2,    
    USER    = 3
};

void stateMachineInit(void);
void stateMachineDestroy(void);

#endif // STATE_MACHINE_H