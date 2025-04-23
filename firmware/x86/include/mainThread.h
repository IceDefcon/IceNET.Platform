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
    MAIN_THREAD_IDLE = 0,
    MAIN_THREAD_NETWORK_ARP_REQUEST,
    MAIN_THREAD_NETWORK_NDP_REQUEST,
    MAIN_THREAD_TCP_TRANSMISSION,
    MAIN_THREAD_UDP_TRANSMISSION,
    MAIN_THREAD_DONE,
    MAIN_THREAD_AMOUNT,
} mainThreadStateType;

typedef struct
{
    mainThreadStateType currentState;
    mainThreadStateType previousState;
    struct task_struct *threadHandle;
    spinlock_t smSpinlock;
    unsigned long irqFlags;
} mainThreadProcess;

/* SET */ void setStateMachine(mainThreadStateType newState);
/* GET */ mainThreadStateType getStateMachine(void);

void mainThreadInit(void);
void mainThreadDestroy(void);

#endif // MAIN_THREAD_H
