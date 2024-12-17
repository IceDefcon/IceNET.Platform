// SPDX-License-Identifier: GPL-2.0-only
/*
 * Ram backed block device driver.
 *
 * Copyright (C) 2007 Nick Piggin
 * Copyright (C) 2007 Novell Inc.
 *
 * Parts derived from drivers/block/rd.c, and drivers/block/loop.c, copyright
 * of their respective owners.
 */

/*!
 *
 * Re-factored: Ice.Marek
 * IceNET Technology 2025
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
#include <linux/mm.h>
#include <linux/page-flags.h>
#include <linux/kernel.h>

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

/**
 *
 * Page size: 4096 bytes (from PAGE_SHIFT = 12)
 * Sector size: 512 bytes (from SECTOR_SHIFT = 9)
 * Sectors per page: 8 sectors (from PAGE_SECTORS_SHIFT = 3)
 * Free batch size: 16 pages (from FREE_BATCH = 16)
 *
 * There are 8 sectors 512 bytes each in one page giving 4096 bytes per page
 *
 * One ramDevice have 32 sectors which gives 4 pages ---> 16KB per ramDisk
 *
 * Total size of used RAM :: 2 devices * 16KB each = gives 32B
 *
 */

#define RAM_DISK_SIZE           32 /* Number of sectors per ramDisk */
#define KERNEL_RAM_DISK_AMOUNT  2  /* Abount of ramDisk's */
#define PAGE_SHIFT              12 /* Standard page size is 4 KB (2^12 = 4096 bytes) */
#define SECTOR_SHIFT            9  /* Standard sector size is 512 bytes (2^9 = 512 bytes) */
#define PAGE_SECTORS_SHIFT      3  /* (PAGE_SHIFT - SECTOR_SHIFT) = 12 - 9 = 3 */
#define PAGE_SECTORS            8  /* 1 << PAGE_SECTORS_SHIFT = 1 << 3 = 8 sectors per page */
#define FREE_BATCH              16 /* Adjust as needed for batch operations */

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

void *ramDiskGetPointer(sector_t sector);
void ramDiskReleasePointer(void *page_data);

int ramDiskInit(void);
void ramDiskDestroy(void);

#endif // RAM_DISK_H
