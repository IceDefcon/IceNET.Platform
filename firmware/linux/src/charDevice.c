/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h> 	// Include for copy_to_user function
#include <linux/slab.h> 	// Include for kmalloc/kfree functions
#include <linux/mutex.h>    // Include for mutex opearations
#include <linux/device.h>   // Include for class_create
#include <linux/fs.h>       // Include for file_operations struct

#include "charDevice.h"
#include "workLoad.h"

MODULE_LICENSE("GPL");

//////////////////////
//                  //
//                  //
//                  //
//    [C] Device    //
//                  //
//                  //
//                  //
//////////////////////

#define  DEVICE_NAME "iceCOM"
#define  CLASS_NAME  "iceCOM"

static int majorNumber;
static struct class*  C_Class  = NULL;
static struct device* C_Device = NULL;
static char message[256] = {0};
static unsigned long size_of_message;
static int numberOpens = 0;
static DEFINE_MUTEX(com_mutex);

static int dev_open(struct inode *inodep, struct file *filep);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static int dev_release(struct inode *inodep, struct file *filep);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

int charDeviceInit(void)
{
    printk(KERN_INFO "[FPGA][ C ] Device Init\n");

    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber<0)
    {
        printk(KERN_ALERT "[FPGA][ C ] Failed to register major number\n");
        return majorNumber;
    }

    C_Class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(C_Class))
    {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "[FPGA][ C ] Failed to register device class\n");
        return PTR_ERR(C_Class);
    }
    
    C_Device = device_create(C_Class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(C_Device))
    {
        class_destroy(C_Class);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "[FPGA][ C ] Failed to create the device\n");
        return PTR_ERR(C_Device);
    }

    mutex_init(&com_mutex);
}

void charDeviceDestroy(void)
{
    device_destroy(C_Class, MKDEV(majorNumber, 0));
    class_unregister(C_Class);
    class_destroy(C_Class);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    mutex_destroy(&com_mutex);
    printk(KERN_INFO "[FPGA][ C ] Device Exit\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
    if(!mutex_trylock(&com_mutex))
    {
        printk(KERN_ALERT "[FPGA][ C ] Device in use by another process");
        return -EBUSY;
    }

    numberOpens++;
    printk(KERN_INFO "[FPGA][ C ] Device has been opened %d time(s)\n", numberOpens);
    return NULL;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
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

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    error_count = copy_from_user(message, buffer, len);

    if(strncmp(message, "a", 1) == 0)
    {
        queue_work(get_fpga_wq(), get_fpga_work());
    }

    if (error_count==0)
    {
        size_of_message = strlen(message);
        printk(KERN_INFO "[FPGA][ C ] Received %d characters from the user\n", len);
        return len;
    } 
    else 
    {
        printk(KERN_INFO "[FPGA][ C ] Failed to receive characters from the user\n");
        return -EFAULT;
    }
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    mutex_unlock(&com_mutex);
    printk(KERN_INFO "[FPGA][ C ] Device successfully closed\n");
    return NULL;
}

#if 0 /* Do I need them ??? */
/* GET */ struct file_operations *get_fops(void)
{
    return &fops;
}

/* GET */ struct mutex *get_com_mutex(void)
{
    return &com_mutex;
}

/* GET */ int *get_majorNumber(void)
{
    return &majorNumber;
}
/* SET */ void set_majorNumber(int major) 
{
	majorNumber = major;
}

/* GET */ struct class *get_C_Class(void)
{
    return &C_Class;
}
/* SET */ void set_C_Class(struct class *class) 
{
	C_Class = class;
}

/* GET */ struct device *get_C_Device(void)
{
    return &C_Device;
}
/* SET */ void gst_C_Device(struct device *device) 
{
	C_Device = device;
}
#endif