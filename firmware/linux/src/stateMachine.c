/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h> // For msleep
#include <linux/gpio.h>
#include <linux/mutex.h>

#include "stateMachine.h"
#include "charDevice.h"
#include "watchdog.h"
#include "isrCtrl.h"
#include "spiCtrl.h"
#include "spiWork.h"
#include "ramAxis.h"

/////////////////////////
//                     //
//                     //
//                     //
// [STM] State Machine //
//                     //
//                     //
//                     //
/////////////////////////

static stateMachineProcess Process =
{
    .currentState = SM_IDLE,
    .threadHandle = NULL,
    .stateMutex = __MUTEX_INITIALIZER(Process.stateMutex),
    .dmaStop = false,
};

/* SET */ void setStateMachine(stateMachineType newState)
{
    mutex_lock(&Process.stateMutex);
    Process.currentState = newState;
    mutex_unlock(&Process.stateMutex);
}

/* GET */ stateMachineType getStateMachine(void)
{
    stateMachineType state;
    mutex_lock(&Process.stateMutex);
    state = Process.currentState;
    mutex_unlock(&Process.stateMutex);
    return state;
}

extern bool stopDma;

/* Kernel state machine */
static int stateMachineThread(void *data)
{
    stateMachineType state;

    while (!kthread_should_stop())
    {
        state = getStateMachine();

        switch(state)
        {
            case SM_IDLE:
                /* Nothing here :: Just wait */
                break;

            case SM_DMA:
                printk(KERN_INFO "[CTRL][STM] DMA mode\n");
                /* Init pointers */
                initTransfer(SECTOR_ENGINE);
                initTransfer(SECTOR_BMI160);
                initTransfer(SECTOR_ADXL345);
                /* Prepare DMA Transfer */
                prepareTransfer(SECTOR_ENGINE, true, false);
                prepareTransfer(SECTOR_BMI160, false, false);
                prepareTransfer(SECTOR_ADXL345, false, true);
                /* Destroy life-time pointers */
                destroyTransfer(SECTOR_ENGINE);
                destroyTransfer(SECTOR_BMI160);
                destroyTransfer(SECTOR_ADXL345);
                setStateMachine(SM_SPI);
                break;

            case SM_SPI:
                printk(KERN_INFO "[CTRL][STM] SPI mode\n");
                /* QUEUE :: Execution of transferFpgaInput */
                queue_work(get_transferFpgaInput_wq(), get_transferFpgaInput_work());
                /**
                 *
                 * IMPORTANT
                 *
                 * In here we need to wait for the
                 * DMA Engine to launch the transfer
                 *
                 * Then wait for the transfer to complete
                 * Predefined 22 Bytes of the config data
                 */
                setStateMachine(SM_INTERRUPT);
                break;

            case SM_INTERRUPT:
                printk(KERN_INFO "[CTRL][STM] INTERRUPT mode\n");
                gpio_set_value(GPIO_INTERRUPT_FROM_CPU, 1);
                gpio_set_value(GPIO_INTERRUPT_FROM_CPU, 0);
                setStateMachine(SM_IDLE);
                break;

            case SM_KILL:
                printk(KERN_INFO "[CTRL][STM] KILL mode\n");
                /* QUEUE :: Execution of killApplication */
                queue_work(get_killApplication_wq(), get_killApplication_work());
                printk(KERN_INFO "[CTRL][STM] Back to IDLE mode\n");
                setStateMachine(SM_IDLE);
                break;

            case SM_PRINT:
                /*
                 * [0] :: DMA Engine Config
                 * [1] :: DMA BMI160 Config
                 * [2] :: DMA BMI160 Config
                 */
                initTransfer(SECTOR_ENGINE);
                initTransfer(SECTOR_BMI160);
                initTransfer(SECTOR_ADXL345);
                printSector(SECTOR_ENGINE);
                printSector(SECTOR_BMI160);
                printSector(SECTOR_ADXL345);
                destroyTransfer(SECTOR_ENGINE);
                destroyTransfer(SECTOR_BMI160);
                destroyTransfer(SECTOR_ADXL345);
                setStateMachine(SM_IDLE);
                break;

            case SM_CMD:
                /* TODO */
                setStateMachine(SM_IDLE);
                break;

            default:
                printk(KERN_ERR "[CTRL][STM] Unknown mode\n");
                return -EINVAL; // Proper error code
        }

        /**
         * 
         * Reduce consumption of CPU resources
         * Add a short delay to prevent 
         * busy waiting
         * 
         */
        msleep(10); /* Release 90% of CPU resources */ 

    }

    return 0;
}

void stateMachineInit(void)
{
    setStateMachine(SM_IDLE);

    Process.threadHandle = kthread_create(stateMachineThread, NULL, "iceStateMachine");
    
    if (IS_ERR(Process.threadHandle))
    {
        printk(KERN_ERR "[INIT][STM] Failed to create kernel thread. Error code: %ld\n", PTR_ERR(Process.threadHandle));
    }
    else
    {
        printk(KERN_INFO "[INIT][STM] Created kthread for stateMachineThread\n");
        wake_up_process(Process.threadHandle);
    }
}

void stateMachineDestroy(void)
{
    if (Process.threadHandle)
    {
        kthread_stop(Process.threadHandle);
        Process.threadHandle = NULL;
    }
    printk(KERN_INFO "[DESTROY][STM] Destroy State Machine kthread\n");
}
