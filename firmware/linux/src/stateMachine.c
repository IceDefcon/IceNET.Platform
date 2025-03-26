/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/delay.h> // For msleep
#include <linux/gpio.h>

#include "stateMachine.h"
#include "charDevice.h"
#include "spiDmaCtrl.h"
#include "watchdog.h"
#include "isrCtrl.h"
#include "spiWork.h"
#include "ramAxis.h"
#include "config.h"

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
    .irqFlags = 0,
};

/* SET */ void setStateMachine(stateMachineType newState)
{
    spin_lock_irqsave(&Process.smSpinlock, Process.irqFlags);
    Process.currentState = newState;
    spin_unlock_irqrestore(&Process.smSpinlock, Process.irqFlags);
}

/* GET */ stateMachineType getStateMachine(void)
{
    stateMachineType state;
    spin_lock_irqsave(&Process.smSpinlock, Process.irqFlags);
    state = Process.currentState;
    spin_unlock_irqrestore(&Process.smSpinlock, Process.irqFlags);
    return state;
}

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

            case SM_DMA_SINGLE:
                if(isConfigDone())
                {
                    printk(KERN_INFO "[CTRL][STM] Normal DMA mode\n");
                    configDMASingle();
                    /* Let notiffy FPGA that Perfiperal devices feedback is received */
                    gpio_set_value(GPIO_CONF_DONE_INTERRUPT_FROM_CPU, 1);
                    gpio_set_value(GPIO_CONF_DONE_INTERRUPT_FROM_CPU, 0);
                    setStateMachine(SM_DONE);
                }
                break;

            case SM_FPGA_CONFIG:
                /**
                 *
                 * TODO
                 *
                 * This need refactoring
                 *
                 */
                printk(KERN_INFO "[CTRL][STM] Long Configuration DMA mode\n");
                /* Init pointers */
                initTransfer(SECTOR_ENGINE);
                initTransfer(SECTOR_BMI160_0);
                initTransfer(SECTOR_BMI160_1);
                initTransfer(SECTOR_ADXL345);
                /* Prepare DMA Transfer */
                prepareTransfer(SECTOR_ENGINE, true, false);
                prepareTransfer(SECTOR_BMI160_0, false, false);
                prepareTransfer(SECTOR_BMI160_1, false, false);
                prepareTransfer(SECTOR_ADXL345, false, true);
                /* Destroy life-time pointers */
                destroyTransfer(SECTOR_ENGINE);
                destroyTransfer(SECTOR_BMI160_0);
                destroyTransfer(SECTOR_BMI160_1);
                destroyTransfer(SECTOR_ADXL345);
                /* Switch to SPI/DMA @ Config */
                configDMAPeripherals();
                /* Schedule Work Queue for SPI/DMA transfer */
                setStateMachine(SM_SPI);
                break;

            case SM_DMA_SENSOR:
                printk(KERN_INFO "[CTRL][STM] Sensor Configuration DMA mode\n");
                configDMASensor();
                setStateMachine(SM_DONE);
                break;

            case SM_DMA_FEEDBACK:
                printk(KERN_INFO "[CTRL][STM] Single Byte Feedback Configuration DMA mode\n");
                configDMAFeedback();
                setStateMachine(SM_DONE);
                break;

            case SM_DMA_CLEAR:
                resetLongDma();
                printk(KERN_INFO "[CTRL][ C ] [1] Clear DMA variables used for verification of IMU's config\n");
                setStateMachine(SM_DONE);
                break;

            case SM_SPI:
                printk(KERN_INFO "[CTRL][STM] SPI mode\n");
                /* QUEUE :: Execution of masterTransferPrimary */
                queue_work(get_masterTransferPrimary_wq(), get_masterTransferPrimary_work());
                setStateMachine(SM_OFFLOAD);
                break;

            case SM_OFFLOAD:
                printk(KERN_INFO "[CTRL][STM] Fifo data offload mode\n");
                gpio_set_value(GPIO_SPI_INTERRUPT_FROM_CPU, 1);
                gpio_set_value(GPIO_SPI_INTERRUPT_FROM_CPU, 0);
                setStateMachine(SM_DONE);
                break;

            case SM_PRINT:
                /*
                 * [0] :: DMA Engine Config
                 * [1] :: DMA BMI160 Config
                 * [2] :: DMA BMI160 Config
                 */
                initTransfer(SECTOR_ENGINE);
                initTransfer(SECTOR_BMI160_0);
                initTransfer(SECTOR_BMI160_1);
                initTransfer(SECTOR_ADXL345);
                printSector(SECTOR_ENGINE);
                printSector(SECTOR_BMI160_0);
                printSector(SECTOR_BMI160_1);
                printSector(SECTOR_ADXL345);
                destroyTransfer(SECTOR_ENGINE);
                destroyTransfer(SECTOR_BMI160_0);
                destroyTransfer(SECTOR_BMI160_1);
                destroyTransfer(SECTOR_ADXL345);
                setStateMachine(SM_DONE);
                break;

            case SM_CMD:
                /* TODO */
                setStateMachine(SM_DONE);
                break;

            case SM_DONE:
                printk(KERN_INFO "[CTRL][STM] Process Complete\n");
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
    spin_lock_init(&Process.smSpinlock);
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
