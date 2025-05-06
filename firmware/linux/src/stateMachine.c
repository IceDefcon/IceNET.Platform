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
#include "memory.h"
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

static DECLARE_WAIT_QUEUE_HEAD(stateMachineThreadWaitQueue);

static stateMachineProcess Process =
{
    .currentState = SM_IDLE,
    .previousState = SM_IDLE,
    .stateChanged = false,
    .threadHandle = NULL,
    .irqFlags = 0,
    .threadName = "iceStateMachine",
};

/* SET */ void setStateMachine(stateMachineType newState)
{
    spin_lock_irqsave(&Process.smSpinlock, Process.irqFlags);
    Process.currentState = newState;
    Process.stateChanged = true; /* Signal that something changed */
    spin_unlock_irqrestore(&Process.smSpinlock, Process.irqFlags);
    wake_up_interruptible(&stateMachineThreadWaitQueue); /* Wake up thread */
}

/* GET */ stateMachineType getStateMachine(void)
{
    stateMachineType state;
    spin_lock_irqsave(&Process.smSpinlock, Process.irqFlags);
    state = Process.currentState;
    spin_unlock_irqrestore(&Process.smSpinlock, Process.irqFlags);
    return state;
}

static const char* getStateMachineThreadString(stateMachineType type)
{
    static const char* stateMachineThreadStrings[] =
    {
        "SM_IDLE",
        "SM_DMA_NORMAL",
        "SM_DMA_SENSOR",
        "SM_DMA_SINGLE",
        "SM_DMA_CUSTOM",
        "SM_RAMDISK_CONFIG",
        "SM_RAMDISK_CLEAR",
        "SM_RAMDISK_PRINT",
        "SM_PRIMARY_SPI",
        "SM_FPGA_RESET",
        "SM_SENSOR_CONFIG_DONE",
        "SM_DONE"
    };

    if (type >= 0 && type < SM_AMOUNT)
    {
        return stateMachineThreadStrings[type];
    }
    else
    {
        return "UNKNOWN_SM_STATE";
    }
}

static int stateMachineThread(void *data)
{
    showThreadDiagnostics(Process.threadName);

    while (!kthread_should_stop())
    {
        /* Sleep until stateChanged or stop signal */
        wait_event_interruptible(stateMachineThreadWaitQueue, Process.stateChanged || kthread_should_stop());

        if (kthread_should_stop())
        {
            break;
        }

        /* Clear flag after wake */
        spin_lock_irqsave(&Process.smSpinlock, Process.irqFlags);
        Process.stateChanged = false;
        spin_unlock_irqrestore(&Process.smSpinlock, Process.irqFlags);

        Process.currentState = getStateMachine();

        if (Process.previousState != Process.currentState)
        {
            printk(KERN_INFO "[CTRL][STM] stateMachineThread %d->%d %s\n", Process.previousState, Process.currentState, getStateMachineThreadString(Process.currentState));
            Process.previousState = Process.currentState;
        }

        switch(Process.currentState)
        {
            case SM_IDLE:
                /* Nothing here :: Just wait */
                break;

            case SM_DMA_NORMAL:
                printk(KERN_INFO "[CTRL][STM] Normal DMA mode\n");
                configDMA(SPI_PRIMARY, DMA_CONFIG_NORMAL);
                setStateMachine(SM_DONE);
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

            case SM_DMA_CUSTOM:
                printk(KERN_INFO "[CTRL][STM] Custom Size Feedback Configuration DMA mode\n");
                configDMA(SPI_SECONDARY, DMA_CONFIG_CUSTOM);
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
                setStateMachine(SM_DONE);
                break;

            case SM_FPGA_RESET:
                printk(KERN_INFO "[CTRL][STM] FPGA Reset mode\n");
                gpio_set_value(GPIO_NUMBER_RESET_FPGA, 1);
                gpio_set_value(GPIO_NUMBER_RESET_FPGA, 0);
                setStateMachine(SM_DONE);
                break;

            case SM_SENSOR_CONFIG_DONE:
                if(isConfigDone())
                {
                    /**
                     * Let notiffy FPGA that feedback from
                     * Perfiperal devices connected to FPGA
                     * is received by the kernel successfully
                     */
                    printk(KERN_INFO "[CTRL][STM] Peripherals configured -> Interrupt Vector Ready\n");
                    setStateMachine(SM_PRIMARY_SPI);
                }
                else
                {
                    printk(KERN_INFO "[ERNO][STM] Peripherals not configured !\n");
                    setStateMachine(SM_DONE);

                }

            case SM_DONE:
                printk(KERN_INFO "[CTRL][STM] Process Complete\n");
                setStateMachine(SM_IDLE);
                break;

            default:
                printk(KERN_ERR "[CTRL][STM] Unknown mode\n");
                return -EINVAL; // Proper error code
        }
    }

    return 0;
}

void stateMachineInit(void)
{
    spin_lock_init(&Process.smSpinlock);
    setStateMachine(SM_IDLE);

    Process.threadHandle = kthread_create(stateMachineThread, NULL, Process.threadName);
    
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
