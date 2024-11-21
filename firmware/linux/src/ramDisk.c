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
    struct brd_device *blockRamDisk;
    struct brd_device *blockRamDiskNext;

    /* Register Kernel Block Device with Major Number 1 :: For Ram0 and Ram1 Disks */
    if (register_blkdev(RAMDISK_MAJOR, KERNEL_BLOCK_DEVICE))
    {
        printk(KERN_ALERT "[INIT][RAM] Unable to register %s\n", KERNEL_BLOCK_DEVICE);
        // return -EIO; /* TODO :: This should be here */
    }
    else
    {
        printk(KERN_ALERT "[INIT][RAM] Register %s\n", KERNEL_BLOCK_DEVICE);
    }

    // brd_check_and_reset_par(); /* TODO :: No check and reset at this time */

    for (i = 0; i < DEVICE_RAM_DISK_AMOUNT; i++)
    {
        blockRamDisk = brd_alloc(i);
        if (!blockRamDisk)
        {
            goto out_free;
        }
        list_add_tail(&blockRamDisk->brd_list, &brd_devices);
    }

out_free:
    list_for_each_entry_safe(blockRamDisk, blockRamDiskNext, &brd_devices, brd_list) {
        list_del(&blockRamDisk->brd_list);
        brd_free(blockRamDisk);
    }
    unregister_blkdev(RAMDISK_MAJOR, "ramdisk");

    pr_info("brd: module NOT loaded !!!\n");
    return -ENOMEM;
}

void ramDiskDestroy(void)
{
    unregister_blkdev(RAMDISK_MAJOR, KERNEL_BLOCK_DEVICE);
    printk(KERN_ALERT "[DESTROY][RAM] %s Destroyed\n", KERNEL_BLOCK_DEVICE);
}
