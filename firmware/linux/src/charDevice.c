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

#include "stateMachine.h"
#include "charDevice.h"
#include "spiWork.h"

//////////////////////
//                  //
//                  //
//                  //
//   [ C ] Device   //
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

static void init_charDevice_Data(void)
{
    charDevice_Data.RxData = NULL;
    charDevice_Data.TxData = NULL;
    charDevice_Data.length = 0;
    charDevice_Data.ready = false;

    printk(KERN_ALERT "[INIT][ C ] Initialize charDevice Data\n");
}

/* GET TRANSFER RX DATA */ struct charDevice_RxData* charDevice_getRxData(void) 
{
    return &charDevice_Data;
}

void charDeviceInit(void)
{
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber<0)
    {
        printk(KERN_ALERT "[INIT][ C ] Failed to register major number: %d\n", majorNumber);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register major number for char Device: %d\n", majorNumber);
    }

    C_Class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(C_Class))
    {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "[INIT][ C ] Failed to register device class: %ld\n", PTR_ERR(C_Class));
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register device class\n");
    }
    
    C_Device = device_create(C_Class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(C_Device))
    {
        class_destroy(C_Class);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "[INIT][ C ] Failed to create the device\n");
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Create char Device\n");
    }

    init_charDevice_Data();

    printk(KERN_ALERT "[INIT][ C ] Lock on Char Device Device Mutex\n");
    mutex_init(&com_mutex);
}

void charDeviceDestroy(void)
{
    device_destroy(C_Class, MKDEV(majorNumber, 0));
    class_unregister(C_Class);
    class_destroy(C_Class);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    mutex_destroy(&com_mutex);
    printk(KERN_INFO "[DESTROY][ C ] Destroy char device\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
    if(!mutex_trylock(&com_mutex))
    {
        printk(KERN_ALERT "[CTRL][ C ] Device in use by another process");
        return -EBUSY;
    }

    numberOpens++;
    printk(KERN_INFO "[CTRL][ C ] Device has been opened %d time(s)\n", numberOpens);

    return CD_OK;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;

    /**
     * 
     * TODO
     * 
     * At the moment no data feedback
     * is computed back to user space
     * 
     */
    charDevice_TxData.data = 0;
    charDevice_TxData.length = 0;

    /* Copy to user space :: *to, *from, size */
    error_count = copy_to_user(buffer, charDevice_TxData.data, charDevice_TxData.length);

    if (0 == error_count)
    {
        printk(KERN_INFO "[CTRL][ C ] Sent %d characters to user-space\n", charDevice_TxData.length);
        /* Clear the position to the start and return NULL */
        return (charDevice_TxData.length = 0);
    }
    else 
    {
        printk(KERN_INFO "[CTRL][ C ] Failed to send %d characters to user-space\n", error_count);
        /* Failed -- return a bad address message (i.e. -14) */
        return -EFAULT;
    }

    return CD_OK;
}

static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    char *data;
    int error_count = 0;
    size_t i;

    /* Allocate memory for the char array to store each character */
    data = kmalloc(len + 1, GFP_KERNEL);
    if (!data) 
    {
        printk(KERN_ALERT "[CTRL][ C ] Memory allocation failed ");
        return -ENOMEM;
    }

    /* Copy data from user space to kernel space */
    error_count = copy_from_user(data, buffer, len);
    if (error_count != 0) 
    {
        /* Free allocated memory */
        kfree(data);
        /* Copy failed */
        return -EFAULT;
    }

    /* Null-terminate the char array */
    data[len] = '\0';

    /* Update charDevice_Data */
    charDevice_Data.RxData = data;
    charDevice_Data.length = len;
    charDevice_Data.ready = true;

    // Print each character of the data array
    for (i = 0; i < charDevice_Data.length; i++) 
    {
        printk(KERN_INFO "[CTRL][ C ] Received Byte[%zu]: 0x%02x\n", i, (unsigned char)charDevice_Data.RxData[i]);
    }

    setStateMachine(SPI);

    return CD_OK;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    mutex_unlock(&com_mutex);
    printk(KERN_INFO "[CTRL][ C ] Device successfully closed\n");
    return CD_OK;
}
