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
#include "spiWork.h"
#include "spiCtrl.h"
#include "gpioCtrl.h"

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
    /* Initialise charDevice */
    charDeviceInit();
    /* Initialise SPI */
    spiInit();
    /* Initialise SPI workload */
    spiWorkInit();
    /* Initialise gpio ISR */
    gpioInit();

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
    gpioDestroy();
    spiWorkDestroy();
    spiDestroy();
    charDeviceDestroy();
    stateMachineDestroy();
}

module_init(fpga_driver_init);
module_exit(fpga_driver_exit);
