/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include <linux/init.h>
#include <linux/module.h>

#include "kernelComms.h"
#include "diagnostics.h"
#include "mainThread.h"
#include "x86network.h"

MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice.Marek");
MODULE_DESCRIPTION("Master x86 Controler");

//////////////////////////
//                      //
//        [x-86]        //
//        MASTER        //
//         CTRL         //
//                      //
//////////////////////////
static int __init master_controler_init(void)
{
    int ret = 0;

    printk(KERN_INFO "-------------------------------------------------\n");
    printk(KERN_INFO "[BEGIN] Master x86 Controler                     \n");
    printk(KERN_INFO "-------------------------------------------------\n");

    /* Initialise Active Host List */
    initActiveHostList();
    /* Initialise main thread */
    mainThreadInit();
    /* Initialise Broadcas Transmiter */
    networkInit();
    /* Initialise Main Commander */
    mainCommanderInit();

    printk(KERN_INFO "-------------------------------------------------\n");
    printk(KERN_INFO "[READY] Master x86 Controler Loaded Successfuly  \n");
    printk(KERN_INFO "-------------------------------------------------\n");

    return ret;
}

//////////////////////////
//                      //
//        [x-86]        //
//         CTRL         //
//         EXIT         //
//                      //
//////////////////////////
static void __exit master_controler_exit(void)
{
    printk(KERN_INFO "-------------------------------------------------\n");
    printk(KERN_INFO "[TERMINATE] Termination of Master x86 Controler  \n");
    printk(KERN_INFO "-------------------------------------------------\n");

    /* Destroy everything */
    mainCommanderDestroy();
    networkDestroy();
    mainThreadDestroy();
    cleanupActiveHostList();

    printk(KERN_INFO "[TERMINATE] Master Controler Terminated Successfully\n");
}

module_init(master_controler_init);
module_exit(master_controler_exit);
