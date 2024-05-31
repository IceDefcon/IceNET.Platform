/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <linux/init.h>        // For __init and __exit macros
#include <linux/module.h>      // For module initialization and exit macros

/* Move tcp server to applicaton */
// #include "networkStack.h"
#include "stateMachine.h"
#include "charDevice.h"
#include "spiWork.h"
#include "spiCtrl.h"
#include "isrCtrl.h"

MODULE_VERSION("1.0");
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
    printk(KERN_INFO "----------------------------------\n");
    printk(KERN_INFO "[BEGIN] IceNET CPU & FPGA Platform\n");
    printk(KERN_INFO "----------------------------------\n");

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
    /* Initialise network stack */
    // tcpServerInit();

    printk(KERN_INFO "----------------------------------\n");
    printk(KERN_INFO "[READY] Driver loaded successfuly \n");
    printk(KERN_INFO "----------------------------------\n");

    return 0;
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
    // tcpServerDestroy();
    isrGpioDestroy();
    spiWorkDestroy();
    spiDestroy();
    charDeviceDestroy();
    stateMachineDestroy();

    printk(KERN_INFO "[TERMINATE] Driver terminated successfully\n");
}

module_init(fpga_driver_init);
module_exit(fpga_driver_exit);
