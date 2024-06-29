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

static charDeviceData Device[DEVICE_AMOUNT] =
{
    [DEVICE_INPUT] =
    {
        .majorNumber = 0,
        .deviceClass = NULL,
        .nodeDevice = NULL,
        .openCount = 0,
        .io_mutex = __MUTEX_INITIALIZER(Device[DEVICE_INPUT].io_mutex),
        .io_transfer =
        {
            .RxData = NULL,
            .TxData = NULL,
            .length = 0,
        }
    },

    [DEVICE_OUTPUT] =
    {
        .majorNumber = 0,
        .deviceClass = NULL,
        .nodeDevice = NULL,
        .openCount = 0,
        .io_mutex = __MUTEX_INITIALIZER(Device[DEVICE_OUTPUT].io_mutex),
        .io_transfer =
        {
            .RxData = NULL,
            .TxData = NULL,
            .length = 0,
        }
    },
};

static DEFINE_MUTEX(wait_mutex);

/* INPUT */ static int inputOpen(struct inode *inodep, struct file *filep);
/* INPUT */ static ssize_t inputRead(struct file *, char *, size_t, loff_t *);
/* INPUT */ static ssize_t inputWrite(struct file *, const char *, size_t, loff_t *);
/* INPUT */ static int inputClose(struct inode *inodep, struct file *filep);
/* OUTPUT */ static int outputOpen(struct inode *inodep, struct file *filep);
/* OUTPUT */ static ssize_t outputRead(struct file *, char *, size_t, loff_t *);
/* OUTPUT */ static ssize_t outputWrite(struct file *, const char *, size_t, loff_t *);
/* OUTPUT */ static int outputClose(struct inode *inodep, struct file *filep);

static struct file_operations fops[DEVICE_AMOUNT] =
{
   [DEVICE_INPUT] =
   {
       .open = inputOpen,
       .read = NULL,
       .write = inputWrite,
       .release = inputClose,
   },

   [DEVICE_OUTPUT] =
   {
       .open = outputOpen,
       .read = outputRead,
       .write = outputWrite,
       .release = outputClose,
   }
};

static void charDeviceDataInit(void)
{
    char *inputRxData, *inputTxData;
    char *outputRxData, *outputTxData;

    /* Allocate memory */
    inputRxData = (char *)kmalloc(4 * sizeof(char), GFP_KERNEL);
    inputTxData = (char *)kmalloc(4 * sizeof(char), GFP_KERNEL);
    outputRxData = (char *)kmalloc(4 * sizeof(char), GFP_KERNEL);
    outputTxData = (char *)kmalloc(4 * sizeof(char), GFP_KERNEL);

    /* Check if memory allocation was successful */
    if (!inputRxData || !inputTxData || !outputRxData || !outputTxData)
    {
        printk(KERN_ERR "[INIT][COM] Memory allocation failed\n");
        kfree(inputRxData);
        kfree(inputTxData);
        kfree(outputRxData);
        kfree(outputTxData);
        return;
    }

    Device[DEVICE_INPUT].io_transfer.RxData = inputRxData;
    Device[DEVICE_INPUT].io_transfer.TxData = inputTxData;
    Device[DEVICE_INPUT].io_transfer.length = 4;

    Device[DEVICE_OUTPUT].io_transfer.RxData = outputRxData;
    Device[DEVICE_OUTPUT].io_transfer.TxData = outputTxData;
    Device[DEVICE_OUTPUT].io_transfer.length = 4;

    /* Lock and wait until feedback transfer unlock it */
    mutex_lock(&wait_mutex);

    printk(KERN_INFO "[INIT][COM] Initialize charDevice Data :: Lock the mutex\n");
}

void charDeviceInit(void)
{
    printk(KERN_ALERT "[INIT][NET] Lock on Wait Mutex\n");
    mutex_init(&wait_mutex);

    charDeviceDataInit();

    //
    // KernelInput
    //
    Device[DEVICE_INPUT].majorNumber = register_chrdev(0, INPUT_DEVICE, &fops[DEVICE_INPUT]);
    if (Device[DEVICE_INPUT].majorNumber<0)
    {
        printk(KERN_ALERT "[INIT][COM] Failed to register major number: %d\n", Device[DEVICE_INPUT].majorNumber);
    }
    else
    {
        printk(KERN_ALERT "[INIT][COM] Register major number for char Device: %d\n", Device[DEVICE_INPUT].majorNumber);
    }

    Device[DEVICE_INPUT].deviceClass = class_create(THIS_MODULE, INPUT_CLASS);
    if (IS_ERR(Device[DEVICE_INPUT].deviceClass))
    {
        unregister_chrdev(Device[DEVICE_INPUT].majorNumber, INPUT_DEVICE);
        printk(KERN_ALERT "[INIT][COM] Failed to register device class: %ld\n", PTR_ERR(Device[DEVICE_INPUT].deviceClass));
    }
    else
    {
        printk(KERN_ALERT "[INIT][COM] Register device class\n");
    }

    Device[DEVICE_INPUT].nodeDevice = device_create(Device[DEVICE_INPUT].deviceClass, NULL, MKDEV(Device[DEVICE_INPUT].majorNumber, 0), NULL, INPUT_DEVICE);
    if (IS_ERR(Device[DEVICE_INPUT].nodeDevice))
    {
        class_destroy(Device[DEVICE_INPUT].deviceClass);
        unregister_chrdev(Device[DEVICE_INPUT].majorNumber, INPUT_DEVICE);
        printk(KERN_ALERT "[INIT][COM] Failed to create the device\n");
    }
    else
    {
        printk(KERN_ALERT "[INIT][COM] Create char Device\n");
    }
    //
    // KernelOutput
    //
    Device[DEVICE_OUTPUT].majorNumber = register_chrdev(0, OUTPUT_DEVICE, &fops[DEVICE_OUTPUT]);
    if (Device[DEVICE_OUTPUT].majorNumber < 0)
    {
        printk(KERN_ALERT "[INIT][NET] Failed to register major number: %d\n", Device[DEVICE_OUTPUT].majorNumber);
    }
    else
    {
        printk(KERN_ALERT "[INIT][NET] Register major number for char Device: %d\n", Device[DEVICE_OUTPUT].majorNumber);
    }

    Device[DEVICE_OUTPUT].deviceClass = class_create(THIS_MODULE, OUTPUT_CLASS);
    if (IS_ERR(Device[DEVICE_OUTPUT].deviceClass))
    {
        unregister_chrdev(Device[DEVICE_OUTPUT].majorNumber, OUTPUT_DEVICE);
        printk(KERN_ALERT "[INIT][NET] Failed to register device class: %ld\n", PTR_ERR(Device[DEVICE_OUTPUT].deviceClass));
    }
    else
    {
        printk(KERN_ALERT "[INIT][NET] Register device class\n");
    }

    Device[DEVICE_OUTPUT].nodeDevice = device_create(Device[DEVICE_OUTPUT].deviceClass, NULL, MKDEV(Device[DEVICE_OUTPUT].majorNumber, 0), NULL, OUTPUT_DEVICE);
    if (IS_ERR(Device[DEVICE_OUTPUT].nodeDevice))
    {
        class_destroy(Device[DEVICE_OUTPUT].deviceClass);
        unregister_chrdev(Device[DEVICE_OUTPUT].majorNumber, OUTPUT_DEVICE);
        printk(KERN_ALERT "[INIT][NET] Failed to create the device\n");
    }
    else
    {
        printk(KERN_ALERT "[INIT][NET] Create char Device\n");
    }
}

void charDeviceDestroy(void)
{
    //
    // KernelInput
    //
    if(Device[DEVICE_INPUT].nodeDevice)
    {
        device_destroy(Device[DEVICE_INPUT].deviceClass, MKDEV(Device[DEVICE_INPUT].majorNumber, 0));
        Device[DEVICE_INPUT].nodeDevice = NULL;
        printk(KERN_INFO "[DESTROY][COM] Device destroyed\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][COM] Canot destroy nodeDevice :: It is already NULL !\n");
    }

    if(Device[DEVICE_INPUT].deviceClass)
    {
        class_unregister(Device[DEVICE_INPUT].deviceClass);
        class_destroy(Device[DEVICE_INPUT].deviceClass);
        Device[DEVICE_INPUT].deviceClass = NULL;
        printk(KERN_INFO "[DESTROY][COM] Class destroyed\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][COM] Canot destroy deviceClass :: It is already NULL !\n");
    }

    if(Device[DEVICE_INPUT].majorNumber != 0)
    {
        unregister_chrdev(Device[DEVICE_INPUT].majorNumber, INPUT_DEVICE);
        Device[DEVICE_INPUT].majorNumber = 0;
        printk(KERN_INFO "[DESTROY][COM] Unregistered character device\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][COM] Canot unregister KernelInput Device :: majorNumber is already 0 !\n");
        printk(KERN_INFO "[DESTROY][COM] Device destroyed\n");
    }
    printk(KERN_INFO "[DESTROY][COM] Char device destruction complete\n");

    //
    // KernelOutput
    //
    if(Device[DEVICE_OUTPUT].nodeDevice)
    {
        device_destroy(Device[DEVICE_OUTPUT].deviceClass, MKDEV(Device[DEVICE_OUTPUT].majorNumber, 0));
        Device[DEVICE_OUTPUT].nodeDevice = NULL;
        printk(KERN_INFO "[DESTROY][NET] Device destroyed\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][NET] Canot destroy nodeDevice :: It is already NULL !\n");
    }

    if(Device[DEVICE_OUTPUT].deviceClass)
    {
        class_unregister(Device[DEVICE_OUTPUT].deviceClass);
        class_destroy(Device[DEVICE_OUTPUT].deviceClass);
        Device[DEVICE_OUTPUT].deviceClass = NULL;
        printk(KERN_INFO "[DESTROY][NET] Class destroyed\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][NET] Canot destroy deviceClass :: It is already NULL !\n");
    }

    if(Device[DEVICE_OUTPUT].majorNumber != 0)
    {
        unregister_chrdev(Device[DEVICE_OUTPUT].majorNumber, OUTPUT_DEVICE);
        Device[DEVICE_OUTPUT].majorNumber = 0;
        printk(KERN_INFO "[DESTROY][NET] Unregistered character device\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][NET] Canot unregister KernelOutput Device :: majorNumber is already 0 !\n");
        printk(KERN_INFO "[DESTROY][NET] Device destroyed\n");
    }

    mutex_destroy(&wait_mutex);
    printk(KERN_INFO "[DESTROY][NET] Wait Mutex destroyed\n");
    printk(KERN_INFO "[DESTROY][NET] Char device destruction complete\n");
}

/**
 *
 * KernelInput Interface
 *
 */
static int inputOpen(struct inode *inodep, struct file *filep)
{
    if(!mutex_trylock(&Device[DEVICE_INPUT].io_mutex))
    {
        printk(KERN_ALERT "[CTRL][COM] Device in use by another process");
        return -EBUSY;
    }

    Device[DEVICE_INPUT].openCount++;
    printk(KERN_INFO "[CTRL][COM] Device has been opened %d time(s)\n", Device[DEVICE_INPUT].openCount);

    return 0;
}

#if 0 /* Not in use */
static ssize_t inputRead(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;

    error_count = copy_to_user(buffer, (const void *)Device[DEVICE_INPUT].io_transfer.TxData, Device[DEVICE_INPUT].io_transfer.length);

    if (error_count == 0)
    {
        printk(KERN_INFO "[CTRL][COM] Sent %zu characters to user-space\n", Device[DEVICE_INPUT].io_transfer.length);
        /* Length == Preamble + Null Terminator */
        return Device[DEVICE_INPUT].io_transfer.length;
    }
    else
    {
        printk(KERN_INFO "[CTRL][COM] Failed to send %d characters to user-space\n", error_count);
        /* Failed -- return a bad address message (i.e. -14) */
        return -EFAULT;
    }
}
#endif

static ssize_t inputWrite(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    size_t i;

    /* Copy RxData from user space to kernel space */
    error_count = copy_from_user((void *)Device[DEVICE_INPUT].io_transfer.RxData, buffer, len);
    if (error_count != 0)
    {
        /* Free allocated memory */
        kfree((void *)Device[DEVICE_INPUT].io_transfer.RxData);
        /* Copy failed */
        return -EFAULT;
    }

    /* Kill signal from Application */
    if (Device[DEVICE_INPUT].io_transfer.RxData[0] == 0xDE && Device[DEVICE_INPUT].io_transfer.RxData[1] == 0xAD)
    {
        printk(KERN_INFO "[CHAR][IWR] Kill SIGNAL received from Application\n");
        setStateMachine(KILL_APPLICATION);
        return 0;
    }

    /* 20ms delayed :: Read Enable pulse to FIFO */
    if (Device[DEVICE_INPUT].io_transfer.RxData[0] == 0x12 && Device[DEVICE_INPUT].io_transfer.RxData[1] == 0x34)
    {
        printk(KERN_INFO "[CHAR][IWR] Generate FIFO rd_en from Kernel [long pulse] to be cut in FPGA\n");
        setStateMachine(INTERRUPT);
        return 0;
    }

    Device[DEVICE_INPUT].io_transfer.RxData[len] = '\0';  /* Null terminate the char array */
    Device[DEVICE_INPUT].io_transfer.length = len;

    // Print each character of the RxData array
    for (i = 0; i < Device[DEVICE_INPUT].io_transfer.length; i++)
    {
        printk(KERN_INFO "[CHAR][IWR] Received Byte[%zu]: 0x%02x\n", i, (unsigned char)Device[DEVICE_INPUT].io_transfer.RxData[i]);
    }

    setStateMachine(SPI);

    return 0;
}

static int inputClose(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[INIT][COM] Unlock [C] Device Mutex\n");
    mutex_unlock(&Device[DEVICE_INPUT].io_mutex);
    printk(KERN_INFO "[CTRL][COM] Device successfully closed\n");
    return 0;
}

/**
 *
 * KernelOutput Interface
 *
 */
static int outputOpen(struct inode *inodep, struct file *filep)
{
    if(!mutex_trylock(&Device[DEVICE_OUTPUT].io_mutex))
    {
        printk(KERN_ALERT "[CTRL][NET] Device in use by another process");
        return -EBUSY;
    }

    Device[DEVICE_OUTPUT].openCount++;
    printk(KERN_INFO "[CTRL][NET] Device has been opened %d time(s)\n", Device[DEVICE_OUTPUT].openCount);

    return 0;
}

static ssize_t outputRead(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;

    printk(KERN_INFO "[CTRL][SPI] Application is waiting for Data\n");
    printk(KERN_INFO "[CTRL][SPI] Kernel is waiting for mutex Unlock\n");
    mutex_lock(&wait_mutex);

    error_count = copy_to_user(buffer, (const void *)Device[DEVICE_OUTPUT].io_transfer.RxData, Device[DEVICE_OUTPUT].io_transfer.length);

    if (error_count == 0)
    {
        printk(KERN_INFO "[CTRL][COM] Sent %zu characters to user-space\n", Device[DEVICE_OUTPUT].io_transfer.length);
        /* Length == Preamble + Null Terminator */
        return Device[DEVICE_OUTPUT].io_transfer.length;
    }
    else
    {
        printk(KERN_INFO "[CTRL][COM] Failed to send %d characters to user-space\n", error_count);
        /* Failed -- return a bad address message (i.e. -14) */
        return -EFAULT;
    }
}

static ssize_t outputWrite(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    size_t i;

    /* Copy RxData from user space to kernel space */
    error_count = copy_from_user((void *)Device[DEVICE_OUTPUT].io_transfer.RxData, buffer, len);
    if (error_count != 0)
    {
        /* Free allocated memory */
        kfree((void *)Device[DEVICE_OUTPUT].io_transfer.RxData);
        /* Copy failed */
        return -EFAULT;
    }

    Device[DEVICE_OUTPUT].io_transfer.RxData[len] = '\0';  /* Null terminate the char array */
    Device[DEVICE_OUTPUT].io_transfer.length = len;

    // Print each character of the RxData array
    for (i = 0; i < Device[DEVICE_OUTPUT].io_transfer.length; i++)
    {
        printk(KERN_INFO "[CTRL][COM] Received Byte[%zu]: 0x%02x\n", i, (unsigned char)Device[DEVICE_OUTPUT].io_transfer.RxData[i]);
    }

    return 0;
}

static int outputClose(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[INIT][NET] Unlock [C] Device Mutex\n");
    mutex_unlock(&Device[DEVICE_OUTPUT].io_mutex);
    printk(KERN_INFO "[CTRL][NET] Device successfully closed\n");
    return 0;
}

/* GET */ DataTransfer* getKernelInputTransfer(void)
{
    return &Device[DEVICE_INPUT].io_transfer;
}

/* GET */ DataTransfer* getKernelOutputTransfer(void)
{
    return &Device[DEVICE_OUTPUT].io_transfer;
}

/* SET */ void setFpgaFeedbackTransfer(const DataTransfer* transferData)
{
    mutex_unlock(&wait_mutex);
}

/* SET */ void setkillApplicationTransfer(const DataTransfer* transferData)
{
    if (transferData != NULL)
    {
        Device[DEVICE_OUTPUT].io_transfer = *transferData;
    }
    else
    {
        printk(KERN_INFO "[CTRL][NET] Kill SIGNAL Received :: Unlock the mutex\n");
        // Handle the error, e.g., log it or assert
    }
    mutex_unlock(&wait_mutex);
}