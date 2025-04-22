/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#include <linux/init.h>        // For __init and __exit macros
#include <linux/module.h>      // For module initialization and exit macros

#include "stateMachine.h"
#include "charDevice.h"
#include "spiDmaCtrl.h"
#include "scheduler.h"
#include "watchdog.h"
#include "spiWork.h"
#include "isrCtrl.h"
#include "console.h"
#include "ramDisk.h"
#include "ramAxis.h"
#include "network.h"
#include "memory.h"

MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice.Marek");
MODULE_DESCRIPTION("FPGA Driver");

//////////////////////////
//                      //
//        [FPGA]        //
//        DRIVER        //
//         INIT         //
//                      //
//////////////////////////
static int __init fpga_driver_init(void)
{
    int ret = 0;

    printk(KERN_INFO "----------------------------------\n");
    printk(KERN_INFO "[BEGIN] IceNET CPU & FPGA Platform\n");
    printk(KERN_INFO "----------------------------------\n");

    /* Initialise Memory Allocation Monitor */
    memoryInit();
    /* Initialise ramAxis */
    ramAxisInit();
    /* Initialise ramDisk */
    ret = ramDiskInit();
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
    /* Initialise UART Console */
    consoleInit();
    /* Initialise kthread Watchdog */
    watchdogInit();
    /* Initialize scheduler */
    schedulerInit();
    /* Initialize network stack */
    networkInit();

    printk(KERN_INFO "--------------------------------------\n");
    printk(KERN_INFO "[READY] FPGA Driver loaded successfuly \n");
    printk(KERN_INFO "--------------------------------------\n");

    return ret;
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
    printk(KERN_INFO "-------------------------------------------------\n");
    printk(KERN_INFO "[TERMINATE] Termination of FPGA driver \n");
    printk(KERN_INFO "-------------------------------------------------\n");
    /* Destroy everything */
    networkDestroy();
    schedulerDestroy();
    watchdogDestroy();
    consoleDestroy();
    isrGpioDestroy();
    spiWorkDestroy();
    spiDestroy();
    charDeviceDestroy();
    stateMachineDestroy();
    ramDiskDestroy();
    ramAxisDestroy();
    memoryDestroy();
    printk(KERN_INFO "[TERMINATE] Driver Terminated Successfully\n");
}

module_init(fpga_driver_init);
module_exit(fpga_driver_exit);
