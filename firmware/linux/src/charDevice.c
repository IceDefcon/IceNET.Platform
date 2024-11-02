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

/* KernelInput Interface */
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

/* KernelOutput Interface */
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

static ssize_t watchdogWrite(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    /**
     * Dummy
     *
     * Not used for WATCHDOG Device
     */
    return 0;
}

static int watchdogClose(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[INIT][ C ] Unlock [C] Device Mutex\n");
    mutex_unlock(&Device[DEVICE_WATCHDOG].device_mutex);
    printk(KERN_INFO "[CTRL][ C ] Device successfully closed\n");
    return 0;
}

/* GET */ charDeviceData* getCharDevice(void)
{
    return Device;
}

/* GET */ DataTransfer* getCharDeviceTransfer(charDeviceType charDevice)
{
    return &Device[charDevice].io_transfer;
}
