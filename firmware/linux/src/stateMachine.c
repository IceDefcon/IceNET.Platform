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

static stateMachineProcess Process =
{
    .currentState = IDLE,
    .thread_handle = NULL,
    .state_mutex = __MUTEX_INITIALIZER(Process.state_mutex),
};

/* SET */ void setStateMachine(stateType newState)
{
    mutex_lock(&Process.state_mutex);
    Process.currentState = newState;
    mutex_unlock(&Process.state_mutex);
}

/* GET */ stateType getStateMachine(void)
{
    stateType state;
    mutex_lock(&Process.state_mutex);
    state = Process.currentState;
    mutex_unlock(&Process.state_mutex);
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
                /* QUEUE :: Execution of transferFpgaInput */
                queue_work(get_transferFpgaInput_wq(), get_transferFpgaInput_work());
                setStateMachine(IDLE);
                break;

            case INTERRUPT:
                printk(KERN_INFO "[CTRL][STM] INTERRUPT mode\n");
                gpio_set_value(GPIO_KERNEL_INTERRUPT, 1);
                gpio_set_value(GPIO_KERNEL_INTERRUPT, 0);
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

    Process.thread_handle = kthread_create(StateMachineThread, NULL, "SM thread handle");
    
    if (IS_ERR(Process.thread_handle)) 
    {
        printk(KERN_ERR "[INIT][STM] Failed to create kernel thread. Error code: %ld\n", PTR_ERR(Process.thread_handle));
    }
    else
    {
        printk(KERN_INFO "[INIT][STM] Created kthread for StateMachineThread\n");
        wake_up_process(Process.thread_handle);
    }
}

void stateMachineDestroy(void)
{
    if (Process.thread_handle) 
    {
        kthread_stop(Process.thread_handle);
        Process.thread_handle = NULL;
    }
    printk(KERN_INFO "[DESTROY][STM] Destroy State Machine kthread\n");
}
