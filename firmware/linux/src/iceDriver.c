/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/uaccess.h>

#include <linux/interrupt.h>
#include <linux/workqueue.h> // For workqueue-related functions and macros
#include <linux/slab.h>      // For memory allocation functions like kmalloc

#include "charDevice.h"
#include "workLoad.h"
#include "spiFpga.h"

MODULE_VERSION("2.0");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice Marek");
MODULE_DESCRIPTION("FPGA Comms Driver");

///////////////////
//               //
//               //
//               //
// State Machine //
//               //
//               //
//               //
///////////////////

enum StateMachine 
{
    IDLE    = 0,
    SPI     = 1,
    I2C     = 2,    
    USER    = 3
};

enum StateMachine STATE;

static struct task_struct *thread_handle;

//////////////////////////////////////////////////////////////
//                                                          //
////                                                      ////
//////  C   O   D   E   :::  S   E   C   T   I   O   N  //////
////                                                      ////
//                                                          //
//////////////////////////////////////////////////////////////

///////////////////
//               //
//               //
//               //
// State Machine //
//               //
//               //
//               //
///////////////////


int StateMachineThread(void *data)
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

//////////////////////////
//                      //
//        [FPGA]        //
//        DRIVER        //
//         INIT         //
//                      //
//////////////////////////
static int __init fpga_driver_init(void)
{
    /* Initialise State Machine kthread config */
    thread_handle = kthread_create(StateMachineThread, NULL, "thread_handle");
    if (IS_ERR(thread_handle)) {
        printk(KERN_ERR "Failed to create kernel thread\n");
        return PTR_ERR(thread_handle);
    }
    wake_up_process(thread_handle);

    /* Initialise SPI */
    spiInit();
    /* Initialise Kernel and Fpga SPI operations */
    kernelWorkInit();
    fpgaWorkInit();
    /* Initialise gpio ISR */
    gpioKernelIsrInit();
    /* Initialise charDevice */
    charDeviceInit();

    return NULL;
}

//////////////////////////
//                      //
//        [FPGA]        //
//        DRIVER        //
//         EXIT         //
//                      //
//////////////////////////

static void __exit fpga_driver_exit(void)
{
    /* Destroy everything */
    kernelWorkDestroy();
    fpgaWorkDestroy();
    spiDestroy();
    gpioKernelIsrDestroy();
    charDeviceDestroy();

    if (thread_handle) 
    {
        kthread_stop(thread_handle);
        thread_handle = NULL;
    }
    printk(KERN_INFO "[FPGA][STM] State Machine Exit\n");
}

module_init(fpga_driver_init);
module_exit(fpga_driver_exit);

//////////////////////////////////////////////////////////
//                                                      //
//                                                      //
// I2C :: Interface                                     //
//                                                      //
//                                                      //
//////////////////////////////////////////////////////////
//                                                      //
// Protocol Selectio                                    //
// For I2C ---> CS must be connected to Vdd             //
//                                                      //
// ID = 0x68 when SA0 = GND                             //
// ID = 0x69 when SA0 = Vdd                             //
//                                                      //
//                                                      //
//                                                      //
//                                                      //
//                                                      //
//////////////////////////////////////////////////////////
 
//////////////////////////////////////////////////////////
//                                                      //
//                                                      //
// CAN :: Interface                                     //
//                                                      //
//                                                      //
//////////////////////////////////////////////////////////
//                                                      //
// config-pin P9_24 can                                 //
// config-pin P9_26 can                                 //
// config-pin P9_19 can                                 //
// config-pin P9_20 can                                 //
//                                                      //
// sudo ip link set can0 up type can bitrate 500000     //
// sudo ip link set can1 up type can bitrate 500000     //
// candump can1                                         //
// cansend can0 123#1122334455667788                    //
//                                                      //
// Yellow   :: Rx   :: P9_24                            //
// Orange   :: Tx   :: P9_26                            //
// Grey     :: GND  :: P9_2                             //
// White    :: 3.3V :: P9_4                             //
//                                                      //
// Yellow   :: Rx   :: P9_19                            //
// Green    :: Tx   :: P9_20                            //
// Orange   :: GND  :: P9_1                             //
// Red      :: 3.3V :: P9_3                             //
//                                                      //
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
//                                                      //
//                                                      //
// 4D-PAM5 :: Encoder                                   //
//                                                      //
//                                                      //
//////////////////////////////////////////////////////////
//                                                      //
//                                                      //
// 1Ghz ADC                                             //
//                                                      //
//                                                      //
//                                                      //
//                                                      //
//                                                      //
//                                                      //
//////////////////////////////////////////////////////////