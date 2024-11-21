/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#include <linux/init.h>
#include <linux/initrd.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/major.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/highmem.h>
#include <linux/mutex.h>
#include <linux/radix-tree.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/backing-dev.h>
#include <linux/uaccess.h>

#include "ramDisk.h"

static ramDiskDevice Device[DEVICE_RAM_DISK_AMOUNT] =
{
    [DEVICE_RAM_DISK_0] =
    {
        .ramDiskNumber = 0,
        .ramDiskQueue = NULL,
        .ramDisk = NULL,
        .ramDiskList = LIST_HEAD_INIT(Device[DEVICE_RAM_DISK_0].ramDiskList),
        .ramDiskLock = __SPIN_LOCK_UNLOCKED(Device[DEVICE_RAM_DISK_0].ramDiskLock),
        .ramDiskPages = RADIX_TREE_INIT(Device[DEVICE_RAM_DISK_0].ramDiskPages, 0),
    },

    [DEVICE_RAM_DISK_1] =
    {
        .ramDiskNumber = 1,
        .ramDiskQueue = NULL,
        .ramDisk = NULL,
        .ramDiskList = LIST_HEAD_INIT(Device[DEVICE_RAM_DISK_1].ramDiskList),
        .ramDiskLock = __SPIN_LOCK_UNLOCKED(Device[DEVICE_RAM_DISK_1].ramDiskLock),
        .ramDiskPages = RADIX_TREE_INIT(Device[DEVICE_RAM_DISK_1].ramDiskPages, 0),
    },
};

void ramDiskInit(void)
{
    printk(KERN_ALERT "[INIT][RAM] Succesfully created %s\n", RAM_0_DEVICE);
    printk(KERN_ALERT "[INIT][RAM] Succesfully created %s\n", RAM_1_DEVICE);
}

void ramDiskDestroy(void)
{
    printk(KERN_ALERT "[DESTROY][RAM] %s Destruction complete\n", RAM_0_DEVICE);
    printk(KERN_ALERT "[DESTROY][RAM] %s Destruction complete\n", RAM_1_DEVICE);
}
