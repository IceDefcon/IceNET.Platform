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
    SM_DMA_NORMAL,
    SM_DMA_SENSOR,
    SM_DMA_SINGLE,
    SM_DMA_CUSTOM,
    SM_RAMDISK_CONFIG,
    SM_RAMDISK_CLEAR,
    SM_RAMDISK_PRINT,
    SM_PRIMARY_SPI,
    SM_FPGA_RESET,
    SM_SENSOR_CONFIG_DONE,
    SM_DONE,
    SM_AMOUNT,
} stateMachineType;

typedef struct
{
    stateMachineType currentState;
    stateMachineType previousState;
    bool stateChanged;
    struct task_struct *threadHandle;
    spinlock_t smSpinlock;
    unsigned long irqFlags;
    const char threadName[];
} stateMachineProcess;

/* SET */ void setStateMachine(stateMachineType newState);
/* GET */ stateMachineType getStateMachine(void);

void stateMachineInit(void);
void stateMachineDestroy(void);

#endif // STATE_MACHINE_H
