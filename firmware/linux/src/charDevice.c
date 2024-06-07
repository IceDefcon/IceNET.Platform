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
#include "types.h"

//////////////////////
//                  //
//                  //
//                  //
//   [ x ] Device   //
//                  //
//                  //
//                  //
//////////////////////

#define  DEVICE_COM "iceCOM"
#define  CLASS_COM  "iceCOM"

#define  DEVICE_NET "iceNET"
#define  CLASS_NET  "iceNET"

static int majorNumber[2];
static struct class*  C_Class[2]  = {NULL,NULL};
static struct device* C_Device[2] = {NULL,NULL};
static int numberOpens[2] = {0,0};

static DEFINE_MUTEX(com_mutex);
static DEFINE_MUTEX(net_mutex);

static int dev_open(struct inode *inodep, struct file *filep);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static int dev_release(struct inode *inodep, struct file *filep);

static DataTransfer charDeviceTransfer; 

enum deviceTYPE
{
    ICE_COM,
    ICE_NET
};

static struct file_operations fops[2] =
{
   {
       .open = dev_open,
       .read = dev_read,
       .write = dev_write,
       .release = dev_release,
   },
   {
       .open = dev_open,
       .read = dev_read,
       .write = dev_write,
       .release = dev_release,
   }
};

static void init_charDevice_Data(void)
{
    char *RxData, *TxData;

    /* Allocate memory for RxData */
    RxData = kmalloc(1 + 1, GFP_KERNEL); /* Register + Null terminate */
    if (!RxData) 
    {
        printk(KERN_ALERT "[CTRL][COM] RxData :: Memory allocation failed ");
    }

    /* Allocate memory for TxData */
    TxData = kmalloc(1 + 1, GFP_KERNEL); /* Register + null terminate */
    if (!TxData) 
    {
        printk(KERN_ALERT "[CTRL][COM] TxData :: Memory allocation failed ");
    }

    TxData[0] = 0xBB; /* C Device Preamble */
    TxData[1] = '\0'; /* Null terminator */

    charDeviceTransfer.RxData = RxData;
    charDeviceTransfer.TxData = TxData; /* TODO :: TxData is Dummy 0xBB */
    charDeviceTransfer.length = 2;
    charDeviceTransfer.ready = false;

    printk(KERN_ALERT "[INIT][COM] Initialize charDevice Data\n");
}

/* GET TRANSFER RX DATA */ DataTransfer* charDevice_getRxData(void) 
{
    return &charDeviceTransfer;
}

void charDeviceInit(void)
{
    //
    // iceCOM
    //
    majorNumber[ICE_COM] = register_chrdev(0, DEVICE_COM, &fops[ICE_COM]);
    if (majorNumber[ICE_COM]<0)
    {
        printk(KERN_ALERT "[INIT][COM] Failed to register major number: %d\n", majorNumber[ICE_COM]);
    }
    else
    {
        printk(KERN_ALERT "[INIT][COM] Register major number for char Device: %d\n", majorNumber[ICE_COM]);
    }

    C_Class[ICE_COM] = class_create(THIS_MODULE, CLASS_COM);
    if (IS_ERR(C_Class[ICE_COM]))
    {
        unregister_chrdev(majorNumber[ICE_COM], DEVICE_COM);
        printk(KERN_ALERT "[INIT][COM] Failed to register device class: %ld\n", PTR_ERR(C_Class[ICE_COM]));
    }
    else
    {
        printk(KERN_ALERT "[INIT][COM] Register device class\n");
    }
    
    C_Device[ICE_COM] = device_create(C_Class[ICE_COM], NULL, MKDEV(majorNumber[ICE_COM], 0), NULL, DEVICE_COM);
    if (IS_ERR(C_Device[ICE_COM]))
    {
        class_destroy(C_Class[ICE_COM]);
        unregister_chrdev(majorNumber[ICE_COM], DEVICE_COM);
        printk(KERN_ALERT "[INIT][COM] Failed to create the device\n");
    }
    else
    {
        printk(KERN_ALERT "[INIT][COM] Create char Device\n");
    }

    init_charDevice_Data();

    printk(KERN_ALERT "[INIT][COM] Lock on [C] Device Mutex\n");
    mutex_init(&com_mutex);

    //
    // iceNET
    //
    majorNumber[ICE_NET] = register_chrdev(0, DEVICE_NET, &fops[ICE_NET]);
    if (majorNumber[ICE_NET] < 0)
    {
        printk(KERN_ALERT "[INIT][NET] Failed to register major number: %d\n", majorNumber[ICE_NET]);
    }
    else
    {
        printk(KERN_ALERT "[INIT][NET] Register major number for char Device: %d\n", majorNumber[ICE_NET]);
    }

    C_Class[ICE_NET] = class_create(THIS_MODULE, CLASS_NET);
    if (IS_ERR(C_Class[ICE_NET]))
    {
        unregister_chrdev(majorNumber[ICE_NET], DEVICE_NET);
        printk(KERN_ALERT "[INIT][NET] Failed to register device class: %ld\n", PTR_ERR(C_Class[ICE_NET]));
    }
    else
    {
        printk(KERN_ALERT "[INIT][NET] Register device class\n");
    }
    
    C_Device[ICE_NET] = device_create(C_Class[ICE_NET], NULL, MKDEV(majorNumber[ICE_NET], 0), NULL, DEVICE_NET);
    if (IS_ERR(C_Device[ICE_NET]))
    {
        class_destroy(C_Class[ICE_NET]);
        unregister_chrdev(majorNumber[ICE_NET], DEVICE_NET);
        printk(KERN_ALERT "[INIT][NET] Failed to create the device\n");
    }
    else
    {
        printk(KERN_ALERT "[INIT][NET] Create char Device\n");
    }

    /* TODO :: Need data architecture for NET communication */
    // init_charDevice_Data();

    printk(KERN_ALERT "[INIT][NET] Lock on [C] Device Mutex\n");
    mutex_init(&net_mutex);
}

void charDeviceDestroy(void)
{
    //
    // iceCOM
    //
    if(C_Device[ICE_COM]) 
    {
        device_destroy(C_Class[ICE_COM], MKDEV(majorNumber[ICE_COM], 0));
        C_Device[ICE_COM] = NULL;
        printk(KERN_INFO "[DESTROY][COM] Device destroyed\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][COM] Canot destroy C_Device[ICE_COM] :: It is already NULL !\n");
    }

    if(C_Class[ICE_COM]) 
    {
        class_unregister(C_Class[ICE_COM]);
        class_destroy(C_Class[ICE_COM]);
        C_Class[ICE_COM] = NULL;
        printk(KERN_INFO "[DESTROY][COM] Class destroyed\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][COM] Canot destroy C_Class[ICE_COM] :: It is already NULL !\n");
    }

    if(majorNumber[ICE_COM] != 0) 
    {
        unregister_chrdev(majorNumber[ICE_COM], DEVICE_COM);
        majorNumber[ICE_COM] = 0;
        printk(KERN_INFO "[DESTROY][COM] Unregistered character device\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][COM] Canot unregister iceCOM Device :: majorNumber[ICE_COM] is already 0 !\n");
        printk(KERN_INFO "[DESTROY][COM] Device destroyed\n");
    }

    mutex_destroy(&com_mutex);
    printk(KERN_INFO "[DESTROY][COM] Mutex destroyed\n");
    printk(KERN_INFO "[DESTROY][COM] Char device destruction complete\n");

    //
    // iceNET
    //
    if(C_Device[ICE_NET]) 
    {
        device_destroy(C_Class[ICE_NET], MKDEV(majorNumber[ICE_NET], 0));
        C_Device[ICE_NET] = NULL;
        printk(KERN_INFO "[DESTROY][NET] Device destroyed\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][NET] Canot destroy C_Device[ICE_NET] :: It is already NULL !\n");
    }

    if(C_Class[ICE_NET]) 
    {
        class_unregister(C_Class[ICE_NET]);
        class_destroy(C_Class[ICE_NET]);
        C_Class[ICE_NET] = NULL;
        printk(KERN_INFO "[DESTROY][NET] Class destroyed\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][NET] Canot destroy C_Class[ICE_NET] :: It is already NULL !\n");
    }

    if(majorNumber[ICE_NET] != 0) 
    {
        unregister_chrdev(majorNumber[ICE_NET], DEVICE_NET);
        majorNumber[ICE_NET] = 0;
        printk(KERN_INFO "[DESTROY][NET] Unregistered character device\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][NET] Canot unregister iceNET Device :: majorNumber[ICE_NET] is already 0 !\n");
        printk(KERN_INFO "[DESTROY][NET] Device destroyed\n");
    }

    mutex_destroy(&net_mutex);
    printk(KERN_INFO "[DESTROY][NET] Mutex destroyed\n");
    printk(KERN_INFO "[DESTROY][NET] Char device destruction complete\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
    if(!mutex_trylock(&com_mutex))
    {
        printk(KERN_ALERT "[CTRL][COM] Device in use by another process");
        return -EBUSY;
    }

    numberOpens[ICE_COM]++;
    printk(KERN_INFO "[CTRL][COM] Device has been opened %d time(s)\n", numberOpens[ICE_COM]);

    if(!mutex_trylock(&net_mutex))
    {
        printk(KERN_ALERT "[CTRL][NET] Device in use by another process");
        return -EBUSY;
    }

    numberOpens[ICE_NET]++;
    printk(KERN_INFO "[CTRL][NET] Device has been opened %d time(s)\n", numberOpens[ICE_NET]);

    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;

    /* TODO :: TxData is Dummy 0xBB */
    error_count = copy_to_user(buffer, (const void *)charDeviceTransfer.TxData, charDeviceTransfer.length);

    if (error_count == 0)
    {
        printk(KERN_INFO "[CTRL][COM] Sent %zu characters to user-space\n", charDeviceTransfer.length);
        /* Length == Preamble + Null Terminator */
        return charDeviceTransfer.length; 
    }
    else 
    {
        printk(KERN_INFO "[CTRL][COM] Failed to send %d characters to user-space\n", error_count);
        /* Failed -- return a bad address message (i.e. -14) */
        return -EFAULT;
    }
}

static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    size_t i;

    /* Copy RxData from user space to kernel space */
    error_count = copy_from_user((void *)charDeviceTransfer.RxData, buffer, len);
    if (error_count != 0) 
    {
        /* Free allocated memory */
        kfree((void *)charDeviceTransfer.RxData);
        /* Copy failed */
        return -EFAULT;
    }

#if 1 /* Test :: Read Enable pulse to FIFO */
    if (charDeviceTransfer.RxData[0] == 0x12 && charDeviceTransfer.RxData[1] == 0x34)
    {
        printk(KERN_INFO "[CTRL][COM] Generate FIFO rd_en from Kernel [long pulse] to be cut in FPGA\n");
        setStateMachine(INTERRUPT);
        return 0;
    }
#endif

    charDeviceTransfer.RxData[len] = '\0';  /* Null terminate the char array */
    charDeviceTransfer.length = len;
    charDeviceTransfer.ready = true;

    // Print each character of the RxData array
    for (i = 0; i < charDeviceTransfer.length; i++) 
    {
        printk(KERN_INFO "[CTRL][COM] Received Byte[%zu]: 0x%02x\n", i, (unsigned char)charDeviceTransfer.RxData[i]);
    }

    setStateMachine(SPI);

    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[INIT][COM] Unlock [C] Device Mutex\n");
    mutex_unlock(&com_mutex);
    printk(KERN_INFO "[CTRL][COM] Device successfully closed\n");
    return 0;
}
