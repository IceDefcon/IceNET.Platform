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

#include "stateMachine.h"
#include "charDevice.h"
#include "spiWork.h"
#include "types.h"

//////////////////////
//                  //
//                  //
//                  //
//   [ C ] Device   //
//                  //
//                  //
//                  //
//////////////////////

static DEFINE_MUTEX(wait_mutex);
static DEFINE_MUTEX(watchdog_mutex);



/* INPUT */ static int inputOpen(struct inode *inodep, struct file *filep);
/* INPUT */ static ssize_t inputRead(struct file *, char *, size_t, loff_t *);
/* INPUT */ static ssize_t inputWrite(struct file *, const char *, size_t, loff_t *);
/* INPUT */ static int inputClose(struct inode *inodep, struct file *filep);
/* OUTPUT */ static int outputOpen(struct inode *inodep, struct file *filep);
/* OUTPUT */ static ssize_t outputRead(struct file *, char *, size_t, loff_t *);
/* OUTPUT */ static ssize_t outputWrite(struct file *, const char *, size_t, loff_t *);
/* OUTPUT */ static int outputClose(struct inode *inodep, struct file *filep);
/* WATCHDOG */ static int watchdogOpen(struct inode *inodep, struct file *filep);
/* WATCHDOG */ static ssize_t watchdogRead(struct file *, char *, size_t, loff_t *);
/* WATCHDOG */ static ssize_t watchdogWrite(struct file *, const char *, size_t, loff_t *);
/* WATCHDOG */ static int watchdogClose(struct inode *inodep, struct file *filep);

static charDeviceData Device[DEVICE_AMOUNT] =
{
    [DEVICE_INPUT] =
    {
        .majorNumber = 0,
        .deviceClass = NULL,
        .nodeDevice = NULL,
        .openCount = 0,
        .device_mutex = __MUTEX_INITIALIZER(Device[DEVICE_INPUT].device_mutex),
        .read_Mutex = __MUTEX_INITIALIZER(Device[DEVICE_INPUT].read_Mutex),

        .io_transfer =
        {
            .RxData = NULL,
            .TxData = NULL,
            .length = 0,
        },

        .fops =
        {
            .open = inputOpen,
            .read = inputRead, /* Dummy :: Not used for INPUT Device */
            .write = inputWrite,
            .release = inputClose,
        }
    },

    [DEVICE_OUTPUT] =
    {
        .majorNumber = 0,
        .deviceClass = NULL,
        .nodeDevice = NULL,
        .openCount = 0,
        .device_mutex = __MUTEX_INITIALIZER(Device[DEVICE_OUTPUT].device_mutex),
        .read_Mutex = __MUTEX_INITIALIZER(Device[DEVICE_OUTPUT].read_Mutex),

        .io_transfer =
        {
            .RxData = NULL,
            .TxData = NULL,
            .length = 0,
        },

        .fops =
        {
            .open = outputOpen,
            .read = outputRead,
            .write = outputWrite, /* Dummy :: Not used for OUTPUT Device */
            .release = outputClose,
        }
    },

    [DEVICE_WATCHDOG] =
    {
        .majorNumber = 0,
        .deviceClass = NULL,
        .nodeDevice = NULL,
        .openCount = 0,
        .device_mutex = __MUTEX_INITIALIZER(Device[DEVICE_WATCHDOG].device_mutex),
        .read_Mutex = __MUTEX_INITIALIZER(Device[DEVICE_WATCHDOG].read_Mutex),

        .io_transfer =
        {
            .RxData = NULL,
            .TxData = NULL,
            .length = 0,
        },

        .fops =
        {
            .open = watchdogOpen,
            .read = watchdogRead,
            .write = watchdogWrite,
            .release = watchdogClose,
        }
    },
};

/* CTRL */ void charDeviceMutexCtrl(charDeviceType charDevice, MutexCtrlType mutexCtrl)
{
    switch(mutexCtrl)
    {
        case MUTEX_CTRL_INIT:
        {
            mutex_init(&Device[charDevice].read_Mutex);
            break;
        };

        case MUTEX_CTRL_LOCK:
        {
            mutex_lock(&Device[charDevice].read_Mutex);
            break;
        };

        case MUTEX_CTRL_UNLOCK:
        {
            mutex_unlock(&Device[charDevice].read_Mutex);
            break;
        };

        case MUTEX_CTRL_DESTROY:
        {
            mutex_destroy(&Device[charDevice].read_Mutex);
            break;
        };

        default:
        {
            printk(KERN_ALERT "[CTRL][ C ] Unknown mutex control operation: %d", mutexCtrl);
            break;
        }
    }
}

static void charDeviceDataInit(void)
{
    char *inputRxData, *inputTxData;
    char *outputRxData, *outputTxData;
    char *watchdogRxData, *watchdogTxData;

    /* Allocate memory */
    inputRxData = (char *)kmalloc(IO_BUFFER_SIZE * sizeof(char), GFP_KERNEL);
    inputTxData = (char *)kmalloc(IO_BUFFER_SIZE * sizeof(char), GFP_KERNEL);
    outputRxData = (char *)kmalloc(IO_BUFFER_SIZE * sizeof(char), GFP_KERNEL);
    outputTxData = (char *)kmalloc(IO_BUFFER_SIZE * sizeof(char), GFP_KERNEL);
    watchdogRxData = (char *)kmalloc(IO_BUFFER_SIZE * sizeof(char), GFP_KERNEL);
    watchdogTxData = (char *)kmalloc(IO_BUFFER_SIZE * sizeof(char), GFP_KERNEL);

    /* Check if memory allocation was successful */
    if (!inputRxData || !inputTxData || !outputRxData || !outputTxData)
    {
        printk(KERN_ERR "[INIT][ C ] Memory allocation failed\n");
        kfree(inputRxData);
        kfree(inputTxData);
        kfree(outputRxData);
        kfree(outputTxData);
        kfree(watchdogRxData);
        kfree(watchdogTxData);
        return;
    }
    else
    {
        printk(KERN_INFO "[INIT][ C ] Memory allocated succesfully for all char devices\n");
    }

    Device[DEVICE_INPUT].io_transfer.RxData = inputRxData;
    Device[DEVICE_INPUT].io_transfer.TxData = inputTxData;
    Device[DEVICE_INPUT].io_transfer.length = IO_BUFFER_SIZE;

    Device[DEVICE_OUTPUT].io_transfer.RxData = outputRxData;
    Device[DEVICE_OUTPUT].io_transfer.TxData = outputTxData;
    Device[DEVICE_OUTPUT].io_transfer.length = IO_BUFFER_SIZE;

    Device[DEVICE_WATCHDOG].io_transfer.RxData = watchdogRxData;
    Device[DEVICE_WATCHDOG].io_transfer.TxData = watchdogTxData;
    Device[DEVICE_WATCHDOG].io_transfer.length = IO_BUFFER_SIZE;


    /* Lock and OUTPUT until feedback transfer unlock it */
    printk(KERN_INFO "[INIT][ C ] Lock on OUTPUT and WATCHDOG mutex\n");
    charDeviceMutexCtrl(DEVICE_OUTPUT, MUTEX_CTRL_LOCK);
    charDeviceMutexCtrl(DEVICE_WATCHDOG, MUTEX_CTRL_LOCK);
}

void charDeviceInit(void)
{
    printk(KERN_ALERT "[INIT][ C ] Initialize OUTPUT and WATCHDOG Mutex\n");
    charDeviceMutexCtrl(DEVICE_OUTPUT, MUTEX_CTRL_INIT);
    charDeviceMutexCtrl(DEVICE_WATCHDOG, MUTEX_CTRL_INIT);

    charDeviceDataInit();

    //
    // KernelInput
    //
    Device[DEVICE_INPUT].majorNumber = register_chrdev(0, INPUT_DEVICE, &Device[DEVICE_INPUT].fops);
    if (Device[DEVICE_INPUT].majorNumber<0)
    {
        printk(KERN_ALERT "[INIT][ C ] Failed to register major number for %s :: %d\n", INPUT_DEVICE, Device[DEVICE_INPUT].majorNumber);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register major number for %s :: %d\n", INPUT_DEVICE, Device[DEVICE_INPUT].majorNumber);
    }

    Device[DEVICE_INPUT].deviceClass = class_create(THIS_MODULE, INPUT_CLASS);
    if (IS_ERR(Device[DEVICE_INPUT].deviceClass))
    {
        unregister_chrdev(Device[DEVICE_INPUT].majorNumber, INPUT_DEVICE);
        printk(KERN_ALERT "[INIT][ C ] Failed to register device class for %s :: %ld\n", INPUT_DEVICE, PTR_ERR(Device[DEVICE_INPUT].deviceClass));
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register device class for %s\n", INPUT_DEVICE);
    }

    Device[DEVICE_INPUT].nodeDevice = device_create(Device[DEVICE_INPUT].deviceClass, NULL, MKDEV(Device[DEVICE_INPUT].majorNumber, 0), NULL, INPUT_DEVICE);
    if (IS_ERR(Device[DEVICE_INPUT].nodeDevice))
    {
        class_destroy(Device[DEVICE_INPUT].deviceClass);
        unregister_chrdev(Device[DEVICE_INPUT].majorNumber, INPUT_DEVICE);
        printk(KERN_ALERT "[INIT][ C ] Failed to create the device for %s\n", INPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Succesfully created char Device for %s\n", INPUT_DEVICE);
    }
    //
    // KernelOutput
    //
    Device[DEVICE_OUTPUT].majorNumber = register_chrdev(0, OUTPUT_DEVICE, &Device[DEVICE_OUTPUT].fops);
    if (Device[DEVICE_OUTPUT].majorNumber < 0)
    {
        printk(KERN_ALERT "[INIT][ C ] Failed to register major number for %s :: %d\n", OUTPUT_DEVICE, Device[DEVICE_OUTPUT].majorNumber);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register major number for %s :: %d\n", OUTPUT_DEVICE, Device[DEVICE_OUTPUT].majorNumber);
    }

    Device[DEVICE_OUTPUT].deviceClass = class_create(THIS_MODULE, OUTPUT_CLASS);
    if (IS_ERR(Device[DEVICE_OUTPUT].deviceClass))
    {
        unregister_chrdev(Device[DEVICE_OUTPUT].majorNumber, OUTPUT_DEVICE);
        printk(KERN_ALERT "[INIT][ C ] Failed to register device class for %s :: %ld\n", OUTPUT_DEVICE, PTR_ERR(Device[DEVICE_OUTPUT].deviceClass));
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register device class for %s\n", OUTPUT_DEVICE);
    }

    Device[DEVICE_OUTPUT].nodeDevice = device_create(Device[DEVICE_OUTPUT].deviceClass, NULL, MKDEV(Device[DEVICE_OUTPUT].majorNumber, 0), NULL, OUTPUT_DEVICE);
    if (IS_ERR(Device[DEVICE_OUTPUT].nodeDevice))
    {
        class_destroy(Device[DEVICE_OUTPUT].deviceClass);
        unregister_chrdev(Device[DEVICE_OUTPUT].majorNumber, OUTPUT_DEVICE);
        printk(KERN_ALERT "[INIT][ C ] Failed to create the device for %s\n", OUTPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Succesfully created char Device for %s\n", OUTPUT_DEVICE);
    }
    //
    // Watchdog
    //
    Device[DEVICE_WATCHDOG].majorNumber = register_chrdev(0, WATCHDOG_DEVICE, &Device[DEVICE_WATCHDOG].fops);
    if (Device[DEVICE_WATCHDOG].majorNumber < 0)
    {
        printk(KERN_ALERT "[INIT][ C ] Failed to register major number for %s :: %d\n", WATCHDOG_DEVICE, Device[DEVICE_WATCHDOG].majorNumber);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register major number for %s :: %d\n", WATCHDOG_DEVICE, Device[DEVICE_WATCHDOG].majorNumber);
    }

    Device[DEVICE_WATCHDOG].deviceClass = class_create(THIS_MODULE, WATCHDOG_CLASS);
    if (IS_ERR(Device[DEVICE_WATCHDOG].deviceClass))
    {
        unregister_chrdev(Device[DEVICE_WATCHDOG].majorNumber, WATCHDOG_DEVICE);
        printk(KERN_ALERT "[INIT][ C ] Failed to register device class for %s :: %ld\n", WATCHDOG_DEVICE, PTR_ERR(Device[DEVICE_WATCHDOG].deviceClass));
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register device class for %s\n", WATCHDOG_DEVICE);
    }

    Device[DEVICE_WATCHDOG].nodeDevice = device_create(Device[DEVICE_WATCHDOG].deviceClass, NULL, MKDEV(Device[DEVICE_WATCHDOG].majorNumber, 0), NULL, WATCHDOG_DEVICE);
    if (IS_ERR(Device[DEVICE_WATCHDOG].nodeDevice))
    {
        class_destroy(Device[DEVICE_WATCHDOG].deviceClass);
        unregister_chrdev(Device[DEVICE_WATCHDOG].majorNumber, WATCHDOG_DEVICE);
        printk(KERN_ALERT "[INIT][ C ] Failed to create the device for %s\n", WATCHDOG_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Succesfully created char Device for %s\n", WATCHDOG_DEVICE);
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
        printk(KERN_ALERT "[DESTROY][ C ] %s Device destroyed\n", INPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot destroy %s\n", INPUT_DEVICE);
    }

    if(Device[DEVICE_INPUT].deviceClass)
    {
        class_unregister(Device[DEVICE_INPUT].deviceClass);
        class_destroy(Device[DEVICE_INPUT].deviceClass);
        Device[DEVICE_INPUT].deviceClass = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s Class destroyed\n", INPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot destroy %s Class\n", INPUT_DEVICE);
    }

    if(Device[DEVICE_INPUT].majorNumber != 0)
    {
        unregister_chrdev(Device[DEVICE_INPUT].majorNumber, INPUT_DEVICE);
        Device[DEVICE_INPUT].majorNumber = 0;
        printk(KERN_ALERT "[DESTROY][ C ] Unregistered %s device\n", INPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot unregister %s Device\n", INPUT_DEVICE);
    }
    printk(KERN_ALERT "[DESTROY][ C ] %s device destruction complete\n", INPUT_DEVICE);

    //
    // KernelOutput
    //
    if(Device[DEVICE_OUTPUT].nodeDevice)
    {
        device_destroy(Device[DEVICE_OUTPUT].deviceClass, MKDEV(Device[DEVICE_OUTPUT].majorNumber, 0));
        Device[DEVICE_OUTPUT].nodeDevice = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s Device destroyed\n", OUTPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot destroy %s\n", OUTPUT_DEVICE);
    }

    if(Device[DEVICE_OUTPUT].deviceClass)
    {
        class_unregister(Device[DEVICE_OUTPUT].deviceClass);
        class_destroy(Device[DEVICE_OUTPUT].deviceClass);
        Device[DEVICE_OUTPUT].deviceClass = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s Class destroyed\n", OUTPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot destroy %s Class\n", OUTPUT_DEVICE);
    }

    if(Device[DEVICE_OUTPUT].majorNumber != 0)
    {
        unregister_chrdev(Device[DEVICE_OUTPUT].majorNumber, OUTPUT_DEVICE);
        Device[DEVICE_OUTPUT].majorNumber = 0;
        printk(KERN_ALERT "[DESTROY][ C ] Unregistered %s device\n", OUTPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot unregister %s Device\n", OUTPUT_DEVICE);

    }
    printk(KERN_ALERT "[DESTROY][ C ] %s device destruction complete\n", OUTPUT_DEVICE);

    //
    // Watchdog
    //
    if(Device[DEVICE_WATCHDOG].nodeDevice)
    {
        device_destroy(Device[DEVICE_WATCHDOG].deviceClass, MKDEV(Device[DEVICE_WATCHDOG].majorNumber, 0));
        Device[DEVICE_WATCHDOG].nodeDevice = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s Device destroyed\n", WATCHDOG_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot destroy %s\n", WATCHDOG_DEVICE);
    }

    if(Device[DEVICE_WATCHDOG].deviceClass)
    {
        class_unregister(Device[DEVICE_WATCHDOG].deviceClass);
        class_destroy(Device[DEVICE_WATCHDOG].deviceClass);
        Device[DEVICE_WATCHDOG].deviceClass = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s Class destroyed\n", WATCHDOG_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot destroy %s Class\n", WATCHDOG_DEVICE);
    }

    if(Device[DEVICE_WATCHDOG].majorNumber != 0)
    {
        unregister_chrdev(Device[DEVICE_WATCHDOG].majorNumber, WATCHDOG_DEVICE);
        Device[DEVICE_WATCHDOG].majorNumber = 0;
        printk(KERN_ALERT "[DESTROY][ C ] Unregistered %s device\n", WATCHDOG_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot unregister %s Device\n", WATCHDOG_DEVICE);
    }
    printk(KERN_ALERT "[DESTROY][ C ] %s device destruction complete\n", WATCHDOG_DEVICE);

    charDeviceMutexCtrl(DEVICE_OUTPUT, MUTEX_CTRL_LOCK);
    charDeviceMutexCtrl(DEVICE_WATCHDOG, MUTEX_CTRL_LOCK);

    printk(KERN_INFO "[DESTROY][ C ] OUTPUT and WATCHDOG Mutex destroyed\n");
}

/**
 *
 * KernelInput Interface
 *
 */
static int inputOpen(struct inode *inodep, struct file *filep)
{
    if(!mutex_trylock(&Device[DEVICE_INPUT].device_mutex))
    {
        printk(KERN_ALERT "[CTRL][ C ] Device in use by another process");
        return -EBUSY;
    }

    Device[DEVICE_INPUT].openCount++;
    printk(KERN_INFO "[CTRL][ C ] Device has been opened %d time(s)\n", Device[DEVICE_INPUT].openCount);

    return 0;
}

static ssize_t inputRead(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    /**
     * Dummy
     *
     * Not used for INPUT Device
     */
    return 0;
}

static ssize_t inputWrite(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    int ret = 0;
    size_t i;

    /* Copy RxData from user space to kernel space */
    error_count = copy_from_user((void *)Device[DEVICE_INPUT].io_transfer.RxData, buffer, len);
    if (error_count != 0)
    {
        /* Free allocated memory */
        kfree((void *)Device[DEVICE_INPUT].io_transfer.RxData);
        /* Copy failed */
        ret = -EFAULT;
    }

    if (Device[DEVICE_INPUT].io_transfer.RxData[0] == 0xDE && Device[DEVICE_INPUT].io_transfer.RxData[1] == 0xAD)
    {
        /* Kill signal from Application */
        printk(KERN_INFO "[CTRL][ C ] Kill SIGNAL received from Application\n");
        setStateMachine(KILL_APPLICATION);
    }
    else if (Device[DEVICE_INPUT].io_transfer.RxData[0] == 0x12 && Device[DEVICE_INPUT].io_transfer.RxData[1] == 0x34)
    {
        /* 20ms delayed :: Read Enable pulse to FIFO */
        printk(KERN_INFO "[CTRL][ C ] Generate FIFO rd_en from Kernel [long pulse] to be cut in FPGA\n");
        setStateMachine(INTERRUPT);
    }
    else
    {
        Device[DEVICE_INPUT].io_transfer.RxData[len] = '\0';  /* Null terminate the char array */
        Device[DEVICE_INPUT].io_transfer.length = len;

        // Print each character of the RxData array
        for (i = 0; i < Device[DEVICE_INPUT].io_transfer.length; i++)
        {
            printk(KERN_INFO "[CTRL][ C ] Received Byte[%zu]: 0x%02x\n", i, (unsigned char)Device[DEVICE_INPUT].io_transfer.RxData[i]);
        }

        setStateMachine(SPI);
    }

    return ret;
}

static int inputClose(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[CTRL][ C ] Unlock [C] Device Mutex\n");
    mutex_unlock(&Device[DEVICE_INPUT].device_mutex);
    printk(KERN_INFO "[CTRL][ C ] Device successfully closed\n");
    return 0;
}

/**
 *
 * KernelOutput Interface
 *
 */
static int outputOpen(struct inode *inodep, struct file *filep)
{
    if(!mutex_trylock(&Device[DEVICE_OUTPUT].device_mutex))
    {
        printk(KERN_ALERT "[CTRL][ C ] Device in use by another process");
        return -EBUSY;
    }

    Device[DEVICE_OUTPUT].openCount++;
    printk(KERN_INFO "[CTRL][ C ] Device has been opened %d time(s)\n", Device[DEVICE_OUTPUT].openCount);

    return 0;
}

static ssize_t outputRead(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    int ret = 0;
    size_t i;

    printk(KERN_INFO "[CTRL][ C ] Kernel is waiting for Wait mutex Unlock\n");
    charDeviceMutexCtrl(DEVICE_OUTPUT, MUTEX_CTRL_LOCK);

    error_count = copy_to_user(buffer, (const void *)Device[DEVICE_OUTPUT].io_transfer.TxData, Device[DEVICE_OUTPUT].io_transfer.length);

    if (error_count == 0)
    {
        printk(KERN_INFO "[CTRL][ C ] Sent %zu characters to user-space\n", Device[DEVICE_OUTPUT].io_transfer.length);
        ret = Device[DEVICE_OUTPUT].io_transfer.length;
    }
    else
    {
        printk(KERN_INFO "[CTRL][ C ] Failed to send %d characters to user-space\n", error_count);
        ret = -EFAULT; /* Failed -- return a bad address message (i.e. -14) */
    }

    for (i = 0; i < Device[DEVICE_OUTPUT].io_transfer.length; ++i)
    {
        Device[DEVICE_OUTPUT].io_transfer.TxData[i] = 0x00;
    }
    /* TODO :: Must be considered */
    Device[DEVICE_OUTPUT].io_transfer.length = 1;

    return ret;
}

static ssize_t outputWrite(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    /**
     * Dummy
     *
     * Not used for OUTPUT Device
     */
    return 0;
}

static int outputClose(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[CTRL][ C ] Unlock [C] Device Mutex\n");
    mutex_unlock(&Device[DEVICE_OUTPUT].device_mutex);
    printk(KERN_INFO "[CTRL][ C ] Device successfully closed\n");
    return 0;
}

static int watchdogOpen(struct inode *inodep, struct file *filep)
{
    if(!mutex_trylock(&Device[DEVICE_WATCHDOG].device_mutex))
    {
        printk(KERN_ALERT "[CTRL][ C ] Device in use by another process");
        return -EBUSY;
    }

    Device[DEVICE_WATCHDOG].openCount++;
    printk(KERN_INFO "[CTRL][ C ] Device has been opened %d time(s)\n", Device[DEVICE_WATCHDOG].openCount);

    return 0;
}

static ssize_t watchdogRead(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;

    charDeviceMutexCtrl(DEVICE_WATCHDOG, MUTEX_CTRL_LOCK);

    error_count = copy_to_user(buffer, (const void *)Device[DEVICE_WATCHDOG].io_transfer.TxData, Device[DEVICE_WATCHDOG].io_transfer.length);

    if (error_count == 0)
    {
        /* Length == Preamble + Null Terminator */
        return Device[DEVICE_WATCHDOG].io_transfer.length;
    }
    else
    {
        printk(KERN_INFO "[CTRL][ C ] Failed to send %d characters to user-space\n", error_count);
        /* Failed -- return a bad address message (i.e. -14) */
        return -EFAULT;
    }
}

/* Dummy :: Not used for WATCHDOG Device */
static ssize_t watchdogWrite(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    return 0;
}

static int watchdogClose(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[INIT][ C ] Unlock [C] Device Mutex\n");
    mutex_unlock(&Device[DEVICE_WATCHDOG].device_mutex);
    printk(KERN_INFO "[CTRL][ C ] Device successfully closed\n");
    return 0;
}

/* GET */ DataTransfer* getCharDeviceTransfer(charDeviceType charDevice)
{
    return &Device[charDevice].io_transfer;
}
