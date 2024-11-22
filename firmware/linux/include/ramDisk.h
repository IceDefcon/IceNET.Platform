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
    int brd_number;
    struct request_queue *brd_queue;
    struct gendisk *brd_disk;
    struct list_head brd_list;
    spinlock_t brd_lock;
    struct radix_tree_root brd_pages;
};

int ramDiskInit(void);
void ramDiskDestroy(void);

#endif // RAM_DISK_H
