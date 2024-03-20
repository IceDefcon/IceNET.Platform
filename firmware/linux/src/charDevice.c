/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h> 		// Include for file_operations struct
#include <linux/uaccess.h> 	// Include for copy_to_user function
#include <linux/slab.h> 	// Include for kmalloc/kfree functions

#include "charDevice.h"

MODULE_LICENSE("GPL");

ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    //
    // Copy to user space :: *to, *from, size :: returns 0 on success
    //
    error_count = copy_to_user(buffer, message, size_of_message);
    memset(message, 0, sizeof(message));

    if (error_count==0)
    {
        printk(KERN_INFO "[FPGA][ C ] Sent %d characters to the user\n", size_of_message);
        return (size_of_message = 0);  // clear the position to the start and return NULL
    }
    else 
    {
        printk(KERN_INFO "[FPGA][ C ] Failed to send %d characters to the user\n", error_count);
        return -EFAULT; // Failed -- return a bad address message (i.e. -14)
    }
}