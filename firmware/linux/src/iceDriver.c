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
#include "isrCtrl.h"

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
    printk(KERN_INFO "[           ] --------------------------\n");
    printk(KERN_INFO "[ S T A R T ] IceNET CPU & FPGA Platform\n");
    printk(KERN_INFO "[           ] --------------------------\n");

    /* Initialise kthread State Machine */
    stateMachineInit();
    /* Initialise charDevice */
    charDeviceInit();
    /* Initialise SPI */
    spiInit();
    /* Initialise SPI workload */
    spiWorkInit();
    /* Initialise gpio ISR */
    isrGpioInit();

    printk(KERN_INFO "[  D O N E  ]\n");
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
    isrGpioDestroy();
    spiWorkDestroy();
    spiDestroy();
    charDeviceDestroy();
    stateMachineDestroy();
    printk(KERN_INFO "[           ] \n");
    printk(KERN_INFO "[  E X I T  ] \n");
    printk(KERN_INFO "[           ] \n");
}

module_init(fpga_driver_init);
module_exit(fpga_driver_exit);
