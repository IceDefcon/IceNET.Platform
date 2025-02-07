/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice Marek");
MODULE_DESCRIPTION("IceNET Driver");

//////////////////////////
//                      //
//        [FPGA]        //
//        DRIVER        //
//         INIT         //
//                      //
//////////////////////////

static int __init iceNET_init(void)
{
    printk(KERN_INFO "----------------------------------\n");
    printk(KERN_INFO "[BEGIN] IceNET CPU & FPGA Platform\n");
    printk(KERN_INFO "----------------------------------\n");

    /**
     *
     * CODE
     * CODE
     * CODE
     * CODE
     * CODE
     *
     */

    printk(KERN_INFO "----------------------------------\n");
    printk(KERN_INFO "[READY] Driver loaded successfuly \n");
    printk(KERN_INFO "----------------------------------\n");

    return 0;
}

static void __exit iceNET_exit(void)
{
    /**
     *
     * CODE
     * CODE
     * CODE
     * CODE
     * CODE
     *
     */

    printk(KERN_INFO "[TERMINATE] Driver terminated successfully\n");
}

module_init(iceNET_init);
module_exit(iceNET_exit);
