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
#include "isrCtrl.h"
#include "spiCtrl.h"
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

static DEFINE_MUTEX(state_mutex);

static struct task_struct *thread_handle;
static stateType currentState = IDLE;

/* SET STATE */ void setStateMachine(stateType newState)
{
    mutex_lock(&state_mutex);
    currentState = newState;
    mutex_unlock(&state_mutex);
}

/* GET STATE */ stateType getStateMachine(void)
{
    stateType state;
    mutex_lock(&state_mutex);
    state = currentState;
    mutex_unlock(&state_mutex);
    return state;
}

/* Kernel state machine */
static int StateMachineThread(void *data)
{
    stateType state;
    
    while (!kthread_should_stop()) 
    {
        state = getStateMachine();
        switch(state)
        {
            case IDLE:
                // printk(KERN_INFO "[CTRL][STM] IDLE mode\n");
                break;

            case SPI:
                printk(KERN_INFO "[CTRL][STM] SPI mode\n");
                /* QUEUE :: Execution of transferFromCharDevice */
                queue_work(get_transferFromCharDevice_wq(), get_transferFromCharDevice_work());
                setStateMachine(IDLE);
                break;

            case INTERRUPT:
                printk(KERN_INFO "[CTRL][STM] INTERRUPT mode\n");
                gpio_set_value(GPIO_KERNEL_INTERRUPT, 1);
                gpio_set_value(GPIO_KERNEL_INTERRUPT, 0);
                setStateMachine(IDLE);
                break;

            case FEEDBACK:
                printk(KERN_INFO "[CTRL][STM] FEEDBACK mode\n");
                /* QUEUE :: Execution of feedbackTransferFromFPGA */
                queue_work(get_feedbackTransferFromFPGA_wq(), get_feedbackTransferFromFPGA_work());
                setStateMachine(IDLE);
                break;

            case KILL_APPLICATION:
                printk(KERN_INFO "[CTRL][STM] KILL_APPLICATION mode\n");
                /* QUEUE :: Execution of killApplication */
                queue_work(get_killApplication_wq(), get_killApplication_work());
                printk(KERN_INFO "[CTRL][STM] Back to IDLE mode\n");
                setStateMachine(IDLE);
                break;

            default:
                printk(KERN_INFO "[CTRL][STM] Unknown mode\n");
                return EINVAL;
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