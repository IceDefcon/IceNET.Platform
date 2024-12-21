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
    .currentState = IDLE,
    .threadHandle = NULL,
    .stateMutex = __MUTEX_INITIALIZER(Process.stateMutex),
    .dmaStop = false,
};

/* SET */ void setStateMachine(stateType newState)
{
    mutex_lock(&Process.stateMutex);
    Process.currentState = newState;
    mutex_unlock(&Process.stateMutex);
}

/* GET */ stateType getStateMachine(void)
{
    stateType state;
    mutex_lock(&Process.stateMutex);
    state = Process.currentState;
    mutex_unlock(&Process.stateMutex);
    return state;
}

extern bool stopDma;

/* Kernel state machine */
static int stateMachineThread(void *data)
{
    dmaEngineType dmaEngineStatus;
    stateType state;

    while (!kthread_should_stop())
    {
        state = getStateMachine();

        switch(state)
        {
            case IDLE:

                /**
                 *
                 * TODO :: Temporary solution
                 *
                 * Do not execute @ __exit
                 *
                 */
                if(false == stopDma)
                {
                    /* Check only if FPGA is configured :: Watchdog is Running */
                    if(true == getIndicatorFPGA())
                    {
                        dmaEngineStatus = checkEngine();

                        if(DMA_ENGINE_READY == dmaEngineStatus && Process.dmaStop == false)
                        {
                            setStateMachine(DMA);
                            Process.dmaStop = true;
                        }
                        else if(DMA_ENGINE_STOP == dmaEngineStatus)
                        {
                            Process.dmaStop = false;
                        }
                    }
                    else
                    {
                        // printk(KERN_INFO "[ERNO][STM] FPFA is not sending Watchdog interrupts\n");
                    }
                }

                break;

            case DMA:
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
                /* QUEUE :: Execution of configFpga */
                queue_work(get_configFpga_wq(), get_configFpga_work());
                setStateMachine(IDLE);
                break;
            case SPI:
                printk(KERN_INFO "[CTRL][STM] SPI mode\n");
                /* QUEUE :: Execution of transferFpgaInput */
                queue_work(get_transferFpgaInput_wq(), get_transferFpgaInput_work());
                setStateMachine(IDLE);

#if 1 /* DMA Engine debug */
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
#endif
                break;

            case INTERRUPT:
                printk(KERN_INFO "[CTRL][STM] INTERRUPT mode\n");
                gpio_set_value(GPIO_KERNEL_INTERRUPT, 1);
                gpio_set_value(GPIO_KERNEL_INTERRUPT, 0);
                setStateMachine(IDLE);
                break;

            case KILL:
                printk(KERN_INFO "[CTRL][STM] KILL mode\n");
                /* QUEUE :: Execution of killApplication */
                queue_work(get_killApplication_wq(), get_killApplication_work());
                printk(KERN_INFO "[CTRL][STM] Back to IDLE mode\n");
                setStateMachine(IDLE);
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
    setStateMachine(IDLE);

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
