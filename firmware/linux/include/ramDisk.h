/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2024
 *
 */

#ifndef RAM_DISK_H
#define RAM_DISK_H

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

/////////////////////////
//                     //
//                     //
//                     //
//    [RAM :: DISK]    //
//                     //
//                     //
//                     //
/////////////////////////

#define  KERNEL_BLOCK_DEVICE "iceNetBlock"
#define  KERNEL_RAM_DISK_AMOUNT 2

#define PAGE_SECTORS_SHIFT  (PAGE_SHIFT - SECTOR_SHIFT)
#define PAGE_SECTORS        (1 << PAGE_SECTORS_SHIFT)

#define FREE_BATCH 16

static LIST_HEAD(brd_devices);
static DEFINE_MUTEX(brd_devices_mutex);

struct blockRamDisk
{
    int Number;
    struct request_queue *Queue;
    struct gendisk *Disk;
    struct list_head List;
    spinlock_t Lock;
    struct radix_tree_root Pages;
};

int ramDiskInit(void);
void ramDiskDestroy(void);

#endif // RAM_DISK_H
