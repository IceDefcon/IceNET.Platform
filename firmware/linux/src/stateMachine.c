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

            case SM_DMA_NORMAL:
                if(isConfigDone())
                {
                    printk(KERN_INFO "[CTRL][STM] Normal DMA mode\n");
                    configDMA(SPI_PRIMARY, DMA_CONFIG_NORMAL);
                    /**
                     * Let notiffy FPGA that feedback from
                     * Perfiperal devices connected to FPGA
                     * is received by the kernel successfully
                     */
                    gpio_set_value(GPIO_CONF_DONE_INTERRUPT_FROM_CPU, 1);
                    gpio_set_value(GPIO_CONF_DONE_INTERRUPT_FROM_CPU, 0);
                    setStateMachine(SM_DONE);
                }
                break;

            case SM_DMA_SENSOR:
                printk(KERN_INFO "[CTRL][STM] Sensor Configuration DMA mode\n");
                configDMA(SPI_SECONDARY, DMA_CONFIG_SENSOR);
                setStateMachine(SM_DONE);
                break;

            case SM_DMA_SINGLE:
                printk(KERN_INFO "[CTRL][STM] Single Byte Feedback Configuration DMA mode\n");
                configDMA(SPI_SECONDARY, DMA_CONFIG_SINGLE);
                setStateMachine(SM_DONE);
                break;

            case SM_RAMDISK_CONFIG:
                printk(KERN_INFO "[CTRL][STM] Long Configuration DMA mode\n");
                prepareRamDiskTransfer();
                /* Switch to SPI/DMA @ FPGA Peripherals Config */
                configDMA(SPI_PRIMARY, DMA_CONFIG_RAMDISK);
                /* Schedule Work Queue for SPI/DMA transfer */
                setStateMachine(SM_PRIMARY_SPI);
                break;

            case SM_RAMDISK_CLEAR:
                resetLongDma();
                printk(KERN_INFO "[CTRL][ C ] [1] Clear DMA variables used for verification of IMU's config\n");
                setStateMachine(SM_DONE);
                break;

            case SM_RAMDISK_PRINT:
                printk(KERN_INFO "[CTRL][STM] Ram Disk Print mode\n");
                printRamDiskData();
                setStateMachine(SM_DONE);
                break;

            case SM_PRIMARY_SPI:
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
