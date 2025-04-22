/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */

#ifndef MAIN_THREAD_H
#define MAIN_THREAD_H

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
    SM_NETWORK_INIT,
    SM_NETWORK_TCP,
    SM_NETWORK_UDP,
    SM_NETWORK_REGISTER_ARP,
    SM_NETWORK_REQUEST_ARP,
    SM_DONE,
} stateMachineType;

typedef struct
{
    stateMachineType currentState;
    stateMachineType previousState;
    struct task_struct *threadHandle;
    spinlock_t smSpinlock;
    unsigned long irqFlags;
} mainThreadProcess;

/* SET */ void setStateMachine(stateMachineType newState);
/* GET */ stateMachineType getStateMachine(void);

void mainThreadInit(void);
void mainThreadDestroy(void);

#endif // MAIN_THREAD_H
