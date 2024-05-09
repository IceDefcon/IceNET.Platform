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
    char *RxData, *TxData;

    /* Allocate memory for RxData */
    RxData = kmalloc(1 + 1, GFP_KERNEL); /* Register + Null terminate */
    if (!RxData) 
    {
        printk(KERN_ALERT "[CTRL][ C ] RxData :: Memory allocation failed ");
        return -ENOMEM;
    }

    /* Allocate memory for TxData */
    TxData = kmalloc(1 + 1, GFP_KERNEL); /* Register + null terminate */
    if (!TxData) 
    {
        printk(KERN_ALERT "[CTRL][ C ] TxData :: Memory allocation failed ");
        return -ENOMEM;
    }

    TxData[0] = 0xBB; /* C Device Preamble */
    TxData[1] = '\0'; /* Null terminator */

    charDeviceTransfer.RxData = RxData;
    charDeviceTransfer.TxData = TxData; /* TODO :: TxData is rubish */
    charDeviceTransfer.length = 2;
    charDeviceTransfer.ready = false;

    printk(KERN_ALERT "[INIT][ C ] Initialize charDevice Data\n");
}

/* GET TRANSFER RX DATA */ DataTransfer* charDevice_getRxData(void) 
{
    return &charDeviceTransfer;
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

    printk(KERN_ALERT "[INIT][ C ] Lock on [C] Device Mutex\n");
    mutex_init(&com_mutex);
}

void charDeviceDestroy(void)
{
    if(C_Device) 
    {
        device_destroy(C_Class, MKDEV(majorNumber, 0));
        C_Device = NULL;
        printk(KERN_INFO "[DESTROY][ C ] Device destroyed\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][ C ] Canot destroy C_Device :: It is already NULL !\n");
    }

    if(C_Class) 
    {
        class_unregister(C_Class);
        class_destroy(C_Class);
        C_Class = NULL;
        printk(KERN_INFO "[DESTROY][ C ] Class destroyed\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][ C ] Canot destroy C_Class :: It is already NULL !\n");
    }

    if(majorNumber != 0) 
    {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        majorNumber = 0;
        printk(KERN_INFO "[DESTROY][ C ] Unregistered character device\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][ C ] Canot unregister iceCOM Device :: majorNumber is already 0 !\n");
        printk(KERN_INFO "[DESTROY][ C ] Device destroyed\n");
    }

    mutex_destroy(&com_mutex);
    printk(KERN_INFO "[DESTROY][ C ] Mutex destroyed\n");
    printk(KERN_INFO "[DESTROY][ C ] Char device destruction complete\n");
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

    /* TODO :: TxData is rubish */
    error_count = copy_to_user(buffer, charDeviceTransfer.TxData, charDeviceTransfer.length);

    if (error_count == 0)
    {
        printk(KERN_INFO "[CTRL][ C ] Sent %ld characters to user-space\n", charDeviceTransfer.length);
        /* Length == Preamble + Null Terminator */
        return charDeviceTransfer.length; 
    }
    else 
    {
        printk(KERN_INFO "[CTRL][ C ] Failed to send %d characters to user-space\n", error_count);
        /* Failed -- return a bad address message (i.e. -14) */
        return -EFAULT;
    }
}

static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    size_t i;

    /* Copy RxData from user space to kernel space */
    error_count = copy_from_user(charDeviceTransfer.RxData, buffer, len);
    if (error_count != 0) 
    {
        /* Free allocated memory */
        kfree(charDeviceTransfer.RxData);
        /* Copy failed */
        return -EFAULT;
    }

    if(charDeviceTransfer.RxData == 'rd')
    {
        setStateMachine(GPIO);
        return CD_OK;
    }

    charDeviceTransfer.RxData[len] = '\0';  /* Null terminate the char array */
    charDeviceTransfer.length = len;
    charDeviceTransfer.ready = true;

    // Print each character of the RxData array
    for (i = 0; i < charDeviceTransfer.length; i++) 
    {
        printk(KERN_INFO "[CTRL][ C ] Received Byte[%zu]: 0x%02x\n", i, (unsigned char)charDeviceTransfer.RxData[i]);
    }

    setStateMachine(SPI);

    return CD_OK;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[INIT][ C ] Unlock [C] Device Mutex\n");
    mutex_unlock(&com_mutex);
    printk(KERN_INFO "[CTRL][ C ] Device successfully closed\n");
    return CD_OK;
}
