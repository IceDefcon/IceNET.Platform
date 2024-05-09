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

#include "stateMachine.h"
#include "charDevice.h"
#include "spiWork.h"

/////////////////////////
//                     //
//                     //
//                     //
// [STM] State Machine //
//                     //
//                     //
//                     //
/////////////////////////


/* GET STATE */ struct stateMachine* getStateMachine(void)
{
    return &stateStaus;
}

/* SET STATE */ void setStateMachine(stateType newState)
{
    stateStaus.state = newState;
}

/**
 * 
 * TODO :: Waiting for RTL to be continued
 * 
 * State machine is running but not configured
 * 
 */
static int StateMachineThread(void *data)
{
    while (!kthread_should_stop()) 
    {
        switch(getStateMachine()->state)
        {
            case IDLE:
                // printk(KERN_INFO "[CTRL][STM] IDLE mode\n");
                break;

            case SPI:
                printk(KERN_INFO "[CTRL][STM] SPI mode\n");
                DataTransfer* transfer = charDevice_getRxData();
                if (true == transfer->ready)
                {
                    printk(KERN_INFO "[CTRL][STM] SPI Data Ready\n");
                    transfer->ready = false;
                    queue_work(get_transferFromCharDevice_wq(), get_transferFromCharDevice_work());
                    setStateMachine(IDLE);
                }
                break;

            case GPIO:
                printk(KERN_INFO "[CTRL][STM] GPIO mode\n");
                gpio_set_value(GPIO_KERNEL_INTERRUPT, 1);
                gpio_set_value(GPIO_KERNEL_INTERRUPT, 0);
                setStateMachine(IDLE);
                break;

            case I2C:
                printk(KERN_INFO "[CTRL][STM] I2C mode\n");
                break;

            case DMA:
                printk(KERN_INFO "[CTRL][STM] DMA mode\n");
                break;

            default:
                printk(KERN_INFO "[CTRL][STM] Unknown mode\n");
                return EINVAL;
        }

    }

    return SM_OK;
}

void stateMachineInit(void)
{
    setStateMachine(IDLE);

    thread_handle = kthread_create(StateMachineThread, NULL, "SM thread handle");
    
    if (IS_ERR(thread_handle)) 
    {
        printk(KERN_ERR "[INIT][STM] Failed to create kernel thread. Error code: %ld\n", PTR_ERR(thread_handle));
    }
    else
    {
        printk(KERN_ERR "[INIT][STM] Created kthread for StateMachineThread");
    }
    wake_up_process(thread_handle);
}

void stateMachineDestroy(void)
{
    if (thread_handle) 
    {
        kthread_stop(thread_handle);
        thread_handle = NULL;
    }
    printk(KERN_INFO "[DESTROY][STM] Destroy State Machine kthread\n");
}