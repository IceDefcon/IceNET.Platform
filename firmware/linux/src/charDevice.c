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
static int dev_close(struct inode *inodep, struct file *filep);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   // Do I need close
   .close = dev_close,
};

static void init_transfer_data(void)
{
    read_data.data = NULL;
    read_data.length = 0;
    read_data.ready = false;

    printk(KERN_ALERT "[INIT][ C ] Initialize transfer data\n");
}

/* GET TRANSFER DATA */ struct transfer_data* get_transfer_data(void) 
{
    return &read_data;
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

    init_transfer_data();

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

    /* TODO :: No feedback so far */
    write_data.data = 0;
    write_data.length = 0;

    /* Copy to user space :: *to, *from, size :: returns 0 on success */
    error_count = copy_to_user(buffer, write_data.data, write_data.length);

    if (0 == error_count)
    {
        printk(KERN_INFO "[CTRL][ C ] Sent %d characters to user-space\n", write_data.length);
        /* Clear the position to the start and return NULL */
        return (write_data.length = 0);
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
        /* Memory allocation failed */
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

    /* Update read_data */
    read_data.data = data;
    read_data.length = len;
    read_data.ready = true;

    // Print each character of the data array
    for (i = 0; i < read_data.length; i++) 
    {
        printk(KERN_INFO "[CTRL][ C ] Received Byte[%zu]: 0x%02x\n", i, (unsigned char)read_data.data[i]);
    }

    setStateMachine(SPI);

    return CD_OK;
}

    /**
     * 
     * TODO
     * 
     * 1. Need struct with parameters to update
     * 2. State machine processing
     * 3. Application interface
     * 
     **/
    // if(strncmp(message, "a", 1) == 0)
    // {
    //     queue_work(get_mainFromCharDevice_wq(), get_mainFromCharDevice_work());
    // }

    // if(strncmp(message, "i", 1) == 0)
    // {
    //     queue_work(get_secondFromCharDevice_wq(), get_secondFromCharDevice_work());
    // }

    // if (error_count==0)
    // {
    //     size_of_message = strlen(message);
    //     printk(KERN_INFO "[CTRL][ C ] Received %d characters from user-space\n", len);
    //     return len;
    // } 
    // else 
    // {
    //     printk(KERN_INFO "[CTRL][ C ] Failed to receive characters from user-space\n");
    //     return -EFAULT;
    // }
    
//     return CD_OK;
// }

static int dev_close(struct inode *inodep, struct file *filep)
{
    mutex_unlock(&com_mutex);
    printk(KERN_INFO "[CTRL][ C ] Device successfully closed\n");
    return CD_OK;
}
