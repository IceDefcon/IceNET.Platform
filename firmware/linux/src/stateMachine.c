/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h> // For msleep()

#include "stateMachine.h"

///////////////////
//               //
//               //
//               //
// State Machine //
//               //
//               //
//               //
///////////////////

static int StateMachineThread(void *data)
{
    int counter = 0;

    STATE = IDLE;

    while (!kthread_should_stop()) 
    {
        switch(STATE)
        {
            case IDLE:
                // printk(KERN_INFO "[FPGA][STM] Idle State [%d]\n",counter);
                break;

            case SPI:
                printk(KERN_INFO "[FPGA][STM] SPI State [%d]\n",counter);
                STATE = IDLE;
                break;

            case I2C:
                printk(KERN_INFO "[FPGA][STM] I2C State [%d]\n",counter);
                STATE = IDLE;
                break;

            case USER:
                printk(KERN_INFO "[FPGA][STM] USER State [%d]\n",counter);
                STATE = IDLE;
                break;

            default:
                printk(KERN_INFO "[FPGA][STM] Unknown State [%d]\n",counter);
                break;
        }

        msleep(1000);  // Delay for 1 second
        counter++;
    }

    return 0;
}

int stateMachineInit(void)
{
    thread_handle = kthread_create(StateMachineThread, NULL, "thread_handle");
    if (IS_ERR(thread_handle)) {
        printk(KERN_ERR "Failed to create kernel thread\n");
        return PTR_ERR(thread_handle);
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
    printk(KERN_INFO "[FPGA][STM] State Machine Exit\n");
}