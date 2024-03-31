/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <linux/init.h>        // For __init and __exit macros
#include <linux/module.h>      // For module initialization and exit macros

#include "stateMachine.h"
#include "charDevice.h"
#include "workLoad.h"
#include "spiFpga.h"
#include "gpioIsr.h"

MODULE_VERSION("2.0");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice Marek");
MODULE_DESCRIPTION("FPGA Comms Driver");

//////////////////////////
//                      //
//        [FPGA]        //
//        DRIVER        //
//         INIT         //
//                      //
//////////////////////////
static int __init fpga_driver_init(void)
{
    /* Initialise kthread State Machine */
    stateMachineInit();
    /* Initialise SPI */
    spiInit();
    /* Initialise Kernel SPI operations */
    spiKernelWorkInit();
    /* Initialise Fpga SPI operations */
    spiFpgaWorkInit();
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
    spiKernelWorkDestroy();
    spiFpgaWorkDestroy();
    spiDestroy();
    gpioKernelIsrDestroy();
    charDeviceDestroy();
    stateMachineDestroy();
}

module_init(fpga_driver_init);
module_exit(fpga_driver_exit);
