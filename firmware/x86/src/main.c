/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include <linux/init.h>        // For __init and __exit macros
#include <linux/module.h>      // For module initialization and exit macros

#include "transmiter.h"

MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice.Marek");
MODULE_DESCRIPTION("Master Controler");

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
    printk(KERN_INFO "[BEGIN] IceNET Master Controler                  \n");
    printk(KERN_INFO "-------------------------------------------------\n");

    /* Initialise Broadcas Transmiter */
    broadcastInit();

    printk(KERN_INFO "-------------------------------------------------\n");
    printk(KERN_INFO "[READY] IceNET Controler loaded successfuly      \n");
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
    printk(KERN_INFO "[TERMINATE] Termination of Master Controler      \n");
    printk(KERN_INFO "-------------------------------------------------\n");
    /* Destroy everything */
    broadcastDestroy();


    printk(KERN_INFO "[TERMINATE] Master Controler Terminated Successfully\n");
}

module_init(master_controler_init);
module_exit(master_controler_exit);
