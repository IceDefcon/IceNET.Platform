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
#include "ramDisk.h"

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

/* Kernel state machine */
static int stateMachineThread(void *data)
{
    int i = 0;
    char temp_buff_A[16];
    char temp_buff_B[16];
    char temp_buff_C[16];
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

                if (read_from_ice_disk(0, temp_buff_A, 16) == 0) 
                {
                    for (i = 0; i < 16; i++) 
                    {
                        pr_info("Byte %d: 0x%02x\n", i, temp_buff_A[i]);
                    }
                } 
                else 
                {
                    pr_err("Failed to read from IceDisk\n");
                }

                if (read_from_ice_disk(1, temp_buff_B, 16) == 0) 
                {
                    for (i = 0; i < 16; i++) 
                    {
                        pr_info("Byte %d: 0x%02x\n", i, temp_buff_B[i]);
                    }
                } 
                else 
                {
                    pr_err("Failed to read from IceDisk\n");
                }

                if (read_from_ice_disk(2, temp_buff_C, 16) == 0) 
                {
                    for (i = 0; i < 16; i++) 
                    {
                        pr_info("Byte %d: 0x%02x\n", i, temp_buff_C[i]);
                    }
                } 
                else 
                {
                    pr_err("Failed to read from IceDisk\n");
                }
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
