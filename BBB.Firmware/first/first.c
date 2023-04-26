


#include <linux/init.h>             // Macros used to mark up functions e.g., __init __exit
#include <linux/module.h>           // Core header for loading LKMs into the kernel
#include <linux/kernel.h>           // Contains types, macros, functions for the kernel


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mare Ice");
MODULE_DESCRIPTION("A simple Linux driver.");
MODULE_VERSION("1.0.0");

static char *name = "IceNET";
module_param(name, charp, S_IRUGO); // Param desc. charp = char ptr, S_IRUGO can be read/not changed
MODULE_PARM_DESC(name, "The name to display in /var/log/kern.log");

static int __init IceNET_init(void)
{
   printk(KERN_INFO " Init %s \n", name);
   return 0;
}

static void __exit IceNET_exit(void)
{
   printk(KERN_INFO " Exit %s \n", name);
}

module_init(IceNET_init);
module_exit(IceNET_exit);