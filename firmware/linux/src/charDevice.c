/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h> 	// Include for copy_to_user function
#include <linux/slab.h> 	// Include for kmalloc/kfree functions
#include <linux/mutex.h>    // Include for mutex opearations
#include <linux/device.h>   // Include for class_create
#include <linux/delay.h> // For msleep

#include "stateMachine.h"
#include "charDevice.h"
#include "spiWork.h"
#include "config.h"
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

/* COMMANDER */ static int commanderOpen(struct inode *inodep, struct file *filep);
/* COMMANDER */ static ssize_t commanderRead(struct file *, char *, size_t, loff_t *);
/* COMMANDER */ static ssize_t commanderWrite(struct file *, const char *, size_t, loff_t *);
/* COMMANDER */ static int commanderClose(struct inode *inodep, struct file *filep);

static charDeviceData Device[DEVICE_AMOUNT] =
{
    [DEVICE_INPUT] =
    {
        .majorNumber = 0,
        .deviceClass = NULL,
        .nodeDevice = NULL,
        .openCount = 0,
        .read_Mutex = __MUTEX_INITIALIZER(Device[DEVICE_INPUT].read_Mutex),
        .isLocked = true,
        .tryLock = 0,

        .io_transfer =
        {
            .RxData = NULL,
            .TxData = NULL,
            .size = 0,
        },

        .fops =
        {
            .open = inputOpen,
            .read = inputRead, /* Dummy :: Not used */
            .write = inputWrite,
            .release = inputClose,
        },

        .name = "KernelInput",
        .nameClass = "KernelInputClass",
    },

    [DEVICE_OUTPUT] =
    {
        .majorNumber = 0,
        .deviceClass = NULL,
        .nodeDevice = NULL,
        .openCount = 0,
        .read_Mutex = __MUTEX_INITIALIZER(Device[DEVICE_OUTPUT].read_Mutex),
        .isLocked = true,
        .tryLock = 0,

        .io_transfer =
        {
            .RxData = NULL,
            .TxData = NULL,
            .size = 0,
        },

        .fops =
        {
            .open = outputOpen,
            .read = outputRead,
            .write = outputWrite, /* Dummy :: Not used */
            .release = outputClose,
        },

        .name = "KernelOutput",
        .nameClass = "KernelOutputClass",
    },

    [DEVICE_WATCHDOG] =
    {
        .majorNumber = 0,
        .deviceClass = NULL,
        .nodeDevice = NULL,
        .openCount = 0,
        .read_Mutex = __MUTEX_INITIALIZER(Device[DEVICE_WATCHDOG].read_Mutex),
        .isLocked = true,
        .tryLock = 0,

        .io_transfer =
        {
            .RxData = NULL,
            .TxData = NULL,
            .size = 0,
        },

        .fops =
        {
            .open = watchdogOpen,
            .read = watchdogRead,
            .write = watchdogWrite, /* Dummy :: Not used */
            .release = watchdogClose,
        },

        .name = "KernelWatchdog",
        .nameClass = "KernelWatchdogClass",
    },

    [DEVICE_COMMANDER] =
    {
        .majorNumber = 0,
        .deviceClass = NULL,
        .nodeDevice = NULL,
        .openCount = 0,
        .read_Mutex = __MUTEX_INITIALIZER(Device[DEVICE_COMMANDER].read_Mutex),
        .isLocked = true,
        .tryLock = 0,

        .io_transfer =
        {
            .RxData = NULL,
            .TxData = NULL,
            .size = 0,
        },

        .fops =
        {
            .open = commanderOpen,
            .read = commanderRead,
            .write = commanderWrite,
            .release = commanderClose,
        },

        .name = "KernelCommander",
        .nameClass = "KernelCommanderClass",
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

        case MUTEX_CTRL_TRYLOCK:
        {
            Device[charDevice].tryLock = mutex_trylock(&Device[charDevice].read_Mutex);
            break;
        };

        case MUTEX_CTRL_UNLOCK:
        {
            mutex_unlock(&Device[charDevice].read_Mutex);
            break;
        };

        case MUTEX_CTRL_DESTROY:
        {
            Device[charDevice].tryLock = mutex_trylock(&Device[charDevice].read_Mutex);

            if (Device[charDevice].tryLock  == 0)
            {
                mutex_unlock(&Device[charDevice].read_Mutex);
            }

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

/* CTRL */ void charDeviceLockCtrl(charDeviceType charDevice, CtrlType Ctrl)
{
    if(CTRL_LOCK == Ctrl)
    {
        Device[charDevice].isLocked = true;
    }
    else if(CTRL_UNLOCK == Ctrl)
    {
        Device[charDevice].isLocked = false;
    }
}

/* CHECK */ bool isDeviceLocked(charDeviceType charDevice)
{
    return Device[charDevice].isLocked;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* OPEN */ static int inputOpen(struct inode *inodep, struct file *filep)
{
    charDeviceMutexCtrl(DEVICE_INPUT, MUTEX_CTRL_TRYLOCK);

    if(!Device[DEVICE_INPUT].tryLock)
    {
        printk(KERN_ALERT "[CTRL][ C ] Device in use by another process");
        return -EBUSY;
    }

    Device[DEVICE_INPUT].openCount++;
    printk(KERN_INFO "[CTRL][ C ] DEVICE_INPUT has been opened %d time(s)\n", Device[DEVICE_INPUT].openCount);

    return 0;
}

/* OPEN */ static int outputOpen(struct inode *inodep, struct file *filep)
{
    charDeviceMutexCtrl(DEVICE_OUTPUT, MUTEX_CTRL_TRYLOCK);

    if(!Device[DEVICE_OUTPUT].tryLock)
    {
        printk(KERN_ALERT "[CTRL][ C ] Device in use by another process");
        return -EBUSY;
    }

    Device[DEVICE_OUTPUT].openCount++;
    printk(KERN_INFO "[CTRL][ C ] DEVICE_OUTPUT has been opened %d time(s)\n", Device[DEVICE_OUTPUT].openCount);

    return 0;
}

/* OPEN */ static int watchdogOpen(struct inode *inodep, struct file *filep)
{
    charDeviceMutexCtrl(DEVICE_WATCHDOG, MUTEX_CTRL_TRYLOCK);

    if(!Device[DEVICE_WATCHDOG].tryLock)
    {
        printk(KERN_ALERT "[CTRL][ C ] Device in use by another process");
        return -EBUSY;
    }

    Device[DEVICE_WATCHDOG].openCount++;
    printk(KERN_INFO "[CTRL][ C ] DEVICE_WATCHDOG has been opened %d time(s)\n", Device[DEVICE_WATCHDOG].openCount);

    return 0;
}

/* OPEN */ static int commanderOpen(struct inode *inodep, struct file *filep)
{
    charDeviceMutexCtrl(DEVICE_COMMANDER, MUTEX_CTRL_TRYLOCK);

    if(!Device[DEVICE_COMMANDER].tryLock)
    {
        printk(KERN_ALERT "[CTRL][ C ] Device in use by another process");
        return -EBUSY;
    }

    Device[DEVICE_COMMANDER].openCount++;
    printk(KERN_INFO "[CTRL][ C ] DEVICE_COMMANDER has been opened %d time(s)\n", Device[DEVICE_COMMANDER].openCount);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
        setStateMachine(SM_KILL);
    }
    else if (Device[DEVICE_INPUT].io_transfer.RxData[0] == 0x12 && Device[DEVICE_INPUT].io_transfer.RxData[1] == 0x34)
    {
#if 1
        /* 20ms delayed :: Read Enable pulse to FIFO */
        printk(KERN_INFO "[CTRL][ C ] Generate FIFO rd_en from Kernel [long pulse] to be cut in FPGA\n");
        setStateMachine(SM_INTERRUPT);
#else
        printk(KERN_INFO "[CTRL][ C ] Dead end driver here is left for debuging purpouses :: For the FPGA to receive INT_FROM_CPU on Demand \n");
        setStateMachine(SM_INTERRUPT);
        printk(KERN_INFO "[CTRL][ C ] Dead end driver here is left for debuging purpouses :: To unlock the DEVICE_OUTPUT Mutex to keep the things running\n");
        charDeviceLockCtrl(DEVICE_OUTPUT, CTRL_UNLOCK);
#endif
    }
    else
    {
        Device[DEVICE_INPUT].io_transfer.RxData[len] = '\0';  /* Null terminate the char array */
        Device[DEVICE_INPUT].io_transfer.size = len;

        // Print each character of the RxData array
        for (i = 0; i < Device[DEVICE_INPUT].io_transfer.size; i++)
        {
            printk(KERN_INFO "[CTRL][ C ] Received Byte[%zu]: 0x%02x\n", i, (unsigned char)Device[DEVICE_INPUT].io_transfer.RxData[i]);
        }

#if CHAR_DEVICE_CTRL
        setStateMachine(SM_SPI);
#elif RAM_DISK_CTRL
        printk(KERN_INFO "[CTRL][ C ] NEW Commander !!!\n");
        printk(KERN_INFO "[CTRL][ C ] This is dead end Driver !!!\n");
        printk(KERN_INFO "[CTRL][ C ] Currently charDevice communication is Disabled\n");
        printk(KERN_INFO "[CTRL][ C ] Please check charDevice.c :: Comment at line 362 for more detalis \n");

        /**
         * SPI<->DMA is reconfigured
         *
         * Now RAM Disk is used to transport
         * data from user space to kernel
         *
         * charDevice is here for later usage
         * since we not taking data from charDevice
         * to send over SPI<->DMA the state machine
         * transition is disabled here
         */
#endif
    }

    return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static ssize_t outputRead(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int ret = 0;
    int error_count = 0;
    size_t i;

    printk(KERN_INFO "[CTRL][ C ] Output Device is waiting for flag release\n");
    charDeviceLockCtrl(DEVICE_OUTPUT, CTRL_LOCK);
    while(isDeviceLocked(DEVICE_OUTPUT))
    {
        msleep(10); /* Release 90% of CPU resources */
    }

    error_count = copy_to_user(buffer, (const void *)Device[DEVICE_OUTPUT].io_transfer.TxData, Device[DEVICE_OUTPUT].io_transfer.size);

    if (error_count == 0)
    {
        printk(KERN_INFO "[CTRL][ C ] Sent %zu characters to user-space\n", Device[DEVICE_OUTPUT].io_transfer.size);
        ret = Device[DEVICE_OUTPUT].io_transfer.size;
    }
    else
    {
        printk(KERN_INFO "[CTRL][ C ] Failed to send %d characters to user-space\n", error_count);
        ret = -EFAULT; /* Failed -- return a bad address message (i.e. -14) */
    }

    for (i = 0; i < Device[DEVICE_OUTPUT].io_transfer.size; ++i)
    {
        Device[DEVICE_OUTPUT].io_transfer.TxData[i] = 0x00;
    }
    /* TODO :: Must be considered */
    Device[DEVICE_OUTPUT].io_transfer.size = 1;

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static ssize_t watchdogRead(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;

    charDeviceLockCtrl(DEVICE_WATCHDOG, CTRL_LOCK);
    while(isDeviceLocked(DEVICE_WATCHDOG))
    {
        msleep(10); /* Release 90% of CPU resources */
    }

    error_count = copy_to_user(buffer, (const void *)Device[DEVICE_WATCHDOG].io_transfer.TxData, Device[DEVICE_WATCHDOG].io_transfer.size);

    if (error_count == 0)
    {
        /* size == Preamble + Null Terminator */
        return Device[DEVICE_WATCHDOG].io_transfer.size;
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static ssize_t commanderRead(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int ret = 0;
    int error_count = 0;
    size_t i;

    Device[DEVICE_WATCHDOG].io_transfer.TxData[0] = 0x12;
    Device[DEVICE_WATCHDOG].io_transfer.TxData[1] = 0x13;
    Device[DEVICE_WATCHDOG].io_transfer.TxData[2] = 0x14;
    Device[DEVICE_WATCHDOG].io_transfer.TxData[3] = 0x15;
    Device[DEVICE_WATCHDOG].io_transfer.TxData[4] = 0x17;
    Device[DEVICE_WATCHDOG].io_transfer.TxData[5] = 0x18;
    Device[DEVICE_WATCHDOG].io_transfer.TxData[6] = 0x19;
    Device[DEVICE_WATCHDOG].io_transfer.TxData[7] = 0xAA;
    Device[DEVICE_WATCHDOG].io_transfer.size = 8;

    error_count = copy_to_user(buffer, (const void *)Device[DEVICE_WATCHDOG].io_transfer.TxData, Device[DEVICE_WATCHDOG].io_transfer.size);

    if (error_count == 0)
    {
        printk(KERN_INFO "[CTRL][ C ] Sent %zu characters to user-space\n", Device[DEVICE_WATCHDOG].io_transfer.size);
        ret = Device[DEVICE_WATCHDOG].io_transfer.size;
    }
    else
    {
        printk(KERN_INFO "[CTRL][ C ] Failed to send %d characters to user-space\n", error_count);
        ret = -EFAULT; /* Failed -- return a bad address message (i.e. -14) */
    }

    for (i = 0; i < Device[DEVICE_WATCHDOG].io_transfer.size; ++i)
    {
        Device[DEVICE_WATCHDOG].io_transfer.TxData[i] = 0x00;
    }

    return ret;
}

static ssize_t commanderWrite(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    int i = 0;
    int ret = 0;
#if RAM_DISK_CTRL
    int error_count = 0;

    /* Copy RxData from user space to kernel space */
    error_count = copy_from_user((void *)Device[DEVICE_COMMANDER].io_transfer.RxData, buffer, len);
    if (error_count != 0)
    {
        /* Free allocated memory */
        kfree((void *)Device[DEVICE_COMMANDER].io_transfer.RxData);
        /* Copy failed */
        ret = -EFAULT;
    }
    if (Device[DEVICE_COMMANDER].io_transfer.RxData[0] == 0xC0 && Device[DEVICE_COMMANDER].io_transfer.RxData[1] == 0xF1)
    {
        /* Activate DMA Engine */
        printk(KERN_INFO "[CTRL][ C ] Activate DMA Engine\n");
        setStateMachine(SM_LONG_DMA);
    }
    if (Device[DEVICE_COMMANDER].io_transfer.RxData[0] == 0xAE && Device[DEVICE_COMMANDER].io_transfer.RxData[1] == 0xC0)
    {
        /* Reconfigure DMA Engine */
        printk(KERN_INFO "[CTRL][ C ] Reconfigure DMA Engine\n");
        setStateMachine(SM_DMA);
    }
    else
    {
        printk(KERN_INFO "[CTRL][ C ] Data Received: \n");
        for (i = 0; i < len; i++)
        {
            printk(KERN_INFO "[CTRL][ C ] Byte[%d] %x\n", i, Device[DEVICE_COMMANDER].io_transfer.RxData[i]);
        }
    }
#endif
    return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* CLOSE */ static int inputClose(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[CTRL][ C ] Unlock Mutex DEVICE_INPUT\n");
    charDeviceMutexCtrl(DEVICE_INPUT, MUTEX_CTRL_UNLOCK);
    printk(KERN_INFO "[CTRL][ C ] Close DEVICE_INPUT\n");
    return 0;
}

/* CLOSE */ static int outputClose(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[CTRL][ C ] Unlock Mutex DEVICE_OUTPUT\n");
    charDeviceMutexCtrl(DEVICE_OUTPUT, MUTEX_CTRL_UNLOCK);
    printk(KERN_INFO "[CTRL][ C ] Close DEVICE_OUTPUT\n");
    return 0;
}

/* CLOSE */ static int watchdogClose(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[CTRL][ C ] Unlock Mutex DEVICE_WATCHDOG\n");
    charDeviceMutexCtrl(DEVICE_WATCHDOG, MUTEX_CTRL_UNLOCK);
    printk(KERN_INFO "[CTRL][ C ] Close DEVICE_WATCHDOG\n");
    return 0;
}

/* CLOSE */ static int commanderClose(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[CTRL][ C ] Unlock Mutex DEVICE_COMMANDER\n");
    charDeviceMutexCtrl(DEVICE_COMMANDER, MUTEX_CTRL_UNLOCK);
    printk(KERN_INFO "[CTRL][ C ] Close DEVICE_COMMANDER\n");
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* GET */ charDeviceData* getCharDevice(void)
{
    return Device;
}

/* GET */ DmaTransferType* getCharDeviceTransfer(charDeviceType charDevice)
{
    return &Device[charDevice].io_transfer;
}
