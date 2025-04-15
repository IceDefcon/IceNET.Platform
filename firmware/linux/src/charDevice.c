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
    [DEVICE_WATCHDOG] =
    {
        .majorNumber = 0,
        .deviceClass = NULL,
        .nodeDevice = NULL,
        .openCount = 0,
        .deviceMutex = __MUTEX_INITIALIZER(Device[DEVICE_WATCHDOG].deviceMutex),
        .isLocked = true,
        .tryLock = 0,
        .transferSize = 2,

        .io_transfer =
        {
            .RxData = NULL,
            .TxData = NULL,
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
        .deviceMutex = __MUTEX_INITIALIZER(Device[DEVICE_COMMANDER].deviceMutex),
        .isLocked = true,
        .tryLock = 0,
        .transferSize = 12,

        .io_transfer =
        {
            .RxData = NULL,
            .TxData = NULL,
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
            mutex_init(&Device[charDevice].deviceMutex);
            break;
        };

        case MUTEX_CTRL_LOCK:
        {
            mutex_lock(&Device[charDevice].deviceMutex);
            break;
        };

        case MUTEX_CTRL_TRYLOCK:
        {
            Device[charDevice].tryLock = mutex_trylock(&Device[charDevice].deviceMutex);
            break;
        };

        case MUTEX_CTRL_UNLOCK:
        {
            mutex_unlock(&Device[charDevice].deviceMutex);
            break;
        };

        case MUTEX_CTRL_DESTROY:
        {
            Device[charDevice].tryLock = mutex_trylock(&Device[charDevice].deviceMutex);

            if (Device[charDevice].tryLock  == 0)
            {
                mutex_unlock(&Device[charDevice].deviceMutex);
            }

            mutex_destroy(&Device[charDevice].deviceMutex);

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

static ssize_t watchdogRead(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;

    charDeviceLockCtrl(DEVICE_WATCHDOG, CTRL_LOCK);
    while(isDeviceLocked(DEVICE_WATCHDOG))
    {
        msleep(10); /* Release 90% of CPU resources */
    }

    error_count = copy_to_user(buffer, (const void *)Device[DEVICE_WATCHDOG].io_transfer.TxData, Device[DEVICE_WATCHDOG].transferSize);

    if (error_count == 0)
    {
        return Device[DEVICE_WATCHDOG].transferSize;
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

#if 1
    charDeviceLockCtrl(DEVICE_COMMANDER, CTRL_LOCK);
    while(isDeviceLocked(DEVICE_COMMANDER))
    {
        msleep(10); /* Release 90% of CPU resources */
    }
#endif
    error_count = copy_to_user(buffer, (const void *)Device[DEVICE_COMMANDER].io_transfer.TxData, Device[DEVICE_COMMANDER].transferSize);

    if (error_count == 0)
    {
#if 0
        printk(KERN_INFO "[CTRL][ C ] Sent %zu characters to user-space\n", Device[DEVICE_COMMANDER].transferSize);
#endif
        ret = Device[DEVICE_COMMANDER].transferSize;
    }
    else
    {
        printk(KERN_INFO "[CTRL][ C ] Failed to send %d characters to user-space\n", error_count);
        ret = -EFAULT; /* Failed -- return a bad address message (i.e. -14) */
    }

    for (i = 0; i < Device[DEVICE_COMMANDER].transferSize; ++i)
    {
        Device[DEVICE_COMMANDER].io_transfer.TxData[i] = 0x00;
    }

    return ret;
}

static uint8_t customDmaSize = 1;
uint8_t getCustomDmaSize(void)
{
    return customDmaSize;
}


static ssize_t commanderWrite(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    int i = 0;
    int ret = 0;
    int error_count = 0;

    printk(KERN_INFO "[CTRL][ C ] Enter commanderWrite \n");
    /* Clear the buffer before we receive data from User Space */
    Device[DEVICE_COMMANDER].io_transfer.RxData[0] = 0x00;
    Device[DEVICE_COMMANDER].io_transfer.RxData[1] = 0x00;
    Device[DEVICE_COMMANDER].io_transfer.RxData[2] = 0x00;
    Device[DEVICE_COMMANDER].io_transfer.RxData[3] = 0x00;
    Device[DEVICE_COMMANDER].io_transfer.RxData[4] = 0x00;
    Device[DEVICE_COMMANDER].io_transfer.RxData[5] = 0x00;
    Device[DEVICE_COMMANDER].io_transfer.RxData[6] = 0x00;
    Device[DEVICE_COMMANDER].io_transfer.RxData[7] = 0x00;

    /* Copy RxData from user space to kernel space */
    error_count = copy_from_user((void *)Device[DEVICE_COMMANDER].io_transfer.RxData, buffer, len);
    if (error_count != 0)
    {
        /* Copy failed */
        ret = -EFAULT;
    }
    else if (Device[DEVICE_COMMANDER].io_transfer.RxData[0] == 0x04 && Device[DEVICE_COMMANDER].io_transfer.RxData[1] == 0xA1)
    {
        /* Reconfigure DMA Engine */
        printk(KERN_INFO "[CTRL][ C ] Reconfigure DMA Engine into normal mode\n");
        setStateMachine(SM_DMA_NORMAL);
    }
    else if (Device[DEVICE_COMMANDER].io_transfer.RxData[0] == 0x5E && Device[DEVICE_COMMANDER].io_transfer.RxData[1] == 0x50)
    {
        /* Reconfigure DMA Engine */
        printk(KERN_INFO "[CTRL][ C ] Reconfigure DMA Engine into sensor mode\n");
        setStateMachine(SM_DMA_SENSOR);
    }
    else if (Device[DEVICE_COMMANDER].io_transfer.RxData[0] == 0x51 && Device[DEVICE_COMMANDER].io_transfer.RxData[1] == 0x6E)
    {
        /* Reconfigure DMA Engine */
        printk(KERN_INFO "[CTRL][ C ] Reconfigure DMA Engine into single byte feedback mode\n");
        setStateMachine(SM_DMA_SINGLE);
    }
    else if (Device[DEVICE_COMMANDER].io_transfer.RxData[0] == 0xC5 && Device[DEVICE_COMMANDER].io_transfer.RxData[1] == 0x70)
    {
        /* Reconfigure DMA Engine */
        customDmaSize = Device[DEVICE_COMMANDER].io_transfer.RxData[2];
        printk(KERN_INFO "[CTRL][ C ] Reconfigure DMA Engine into custom %d Byte feedback mode\n", customDmaSize);
        setStateMachine(SM_DMA_CUSTOM);

    }
    else if (Device[DEVICE_COMMANDER].io_transfer.RxData[0] == 0xC0 && Device[DEVICE_COMMANDER].io_transfer.RxData[1] == 0xF1)
    {
        /* Activate DMA Engine */
        printk(KERN_INFO "[CTRL][ C ] Activate DMA transfer to send IMU's config to FPGA\n");
        setStateMachine(SM_RAMDISK_CONFIG);
    }
    else if (Device[DEVICE_COMMANDER].io_transfer.RxData[0] == 0xC1 && Device[DEVICE_COMMANDER].io_transfer.RxData[1] == 0xEA)
    {
        printk(KERN_INFO "[CTRL][ C ] [0] Clear DMA variables used for verification of IMU's config\n");
        setStateMachine(SM_RAMDISK_CLEAR);
    }
    else if (Device[DEVICE_COMMANDER].io_transfer.RxData[0] == 0x4E && Device[DEVICE_COMMANDER].io_transfer.RxData[1] == 0x5E)
    {
        printk(KERN_INFO "[CTRL][ C ] Reset everything in FPGA :: Global Discharge\n");
        setStateMachine(SM_FPGA_RESET);
    }
    else
    {
        printk(KERN_INFO "[CTRL][ C ] Data Received: \n");
        for (i = 0; i < len; i++)
        {
            printk(KERN_INFO "[CTRL][ C ] Byte[%d] %x\n", i, Device[DEVICE_COMMANDER].io_transfer.RxData[i]);
        }
        setStateMachine(SM_PRIMARY_SPI);
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* CLOSE */ static int watchdogClose(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[CTRL][ C ] DEVICE_WATCHDOG Unlock Mutex\n");
    charDeviceMutexCtrl(DEVICE_WATCHDOG, MUTEX_CTRL_UNLOCK);
    printk(KERN_INFO "[CTRL][ C ] DEVICE_WATCHDOG Close\n");
    return 0;
}

/* CLOSE */ static int commanderClose(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[CTRL][ C ] DEVICE_COMMANDER Unlock Mutex\n");
    charDeviceMutexCtrl(DEVICE_COMMANDER, MUTEX_CTRL_UNLOCK);
    printk(KERN_INFO "[CTRL][ C ] DEVICE_COMMANDER Close\n");
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
