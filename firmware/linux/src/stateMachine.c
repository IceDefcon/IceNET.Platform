/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h> // For msleep

#include "stateMachine.h"
#include "charDevice.h"

/////////////////////////
//                     //
//                     //
//                     //
// [STM] State Machine //
//                     //
//                     //
//                     //
/////////////////////////

static struct task_struct *thread_handle;

/**
 * 
 * TODO :: Waiting for RTL to be continued
 * 
 * State machine is running but not configured
 * 
 * Require void pointer due to 
 * kthread init NULL Parameter
 * 
 */
static int StateMachineThread(void *data)
{
    int counter = 0;
    stateMachineType STATE = IDLE;
    struct transfer_data* transfer = get_transfer_data();

    while (!kthread_should_stop()) 
    {
        if (true == transfer->ready)
        {
            printk(KERN_INFO "[CTRL][STM] SPI Data Ready\n");
            transfer->ready = false;
        } 

        switch(STATE)
        {
            case IDLE:
                // Handle IDLE state logic
                break;

            case SPI:
                printk(KERN_INFO "[CTRL][STM] SPI State [%d]\n", counter);
                STATE = IDLE;  // Transition to IDLE state
                break;

            case I2C:
                printk(KERN_INFO "[CTRL][STM] I2C State [%d]\n", counter);
                STATE = IDLE;  // Transition to IDLE state
                break;

            case USER:
                printk(KERN_INFO "[CTRL][STM] USER State [%d]\n", counter);
                STATE = IDLE;  // Transition to IDLE state
                break;

            default:
                printk(KERN_INFO "[CTRL][STM] Unknown State [%d]\n", counter);
                return EINVAL;
        }

        msleep(1000);  // Delay for 1 second
        counter++;
    }

    return SM_OK;
}

void stateMachineInit(void)
{
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