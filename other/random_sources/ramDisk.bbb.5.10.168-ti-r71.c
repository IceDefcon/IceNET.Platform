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
#include "ramDisk.h"

static int max_part = 1;

static blk_qc_t BioRequest(struct bio *bio); /* Indicates a bio request (read or write) has been submitted to the block device */
static int RwPage(struct block_device *bdev, sector_t sector, struct page *page, unsigned int op);

static const struct block_device_operations fops =
{
    .owner = THIS_MODULE,
    .submit_bio = BioRequest,
    .rw_page = RwPage,
};

static struct list_head iceDevices = 
{
    &(iceDevices), /* next */
    &(iceDevices)  /* prev */
};

/*
 * Look up and return a ramDisk's page for a given sector.
 */
static struct page *LookupPage(struct blockRamDisk *ramDisk, sector_t sector)
{
    pgoff_t idx;
    struct page *page;

    /*
     * The page lifetime is protected by the fact that we have opened the
     * device node -- ramDisk pages will never be deleted under us, so we
     * don't need any further locking or refcounting.
     *
     * This is strictly true for the radix-tree nodes as well (ie. we
     * don't actually need the rcu_read_lock()), however that is not a
     * documented feature of the radix-tree API so it is better to be
     * safe here (we don't have total exclusion from radix tree updates
     * here, only deletes).
     */
    rcu_read_lock();
    idx = sector >> PAGE_SECTORS_SHIFT; /* sector to page index */
    page = radix_tree_lookup(&ramDisk->Pages, idx);
    rcu_read_unlock();

    BUG_ON(page && page->index != idx);

    return page;
}

/*
 * Look up and return a ramDisk's page for a given sector.
 * If one does not exist, allocate an empty page, and insert that. Then
 * return it.
 */
static struct page *InsertPage(struct blockRamDisk *ramDisk, sector_t sector)
{
    pgoff_t idx;
    struct page *page;
    gfp_t gfp_flags;

    page = LookupPage(ramDisk, sector);
    if (page)
        return page;

    /*
     * Must use NOIO because we don't want to recurse back into the
     * block or filesystem layers from page reclaim.
     */
    gfp_flags = GFP_NOIO | __GFP_ZERO | __GFP_HIGHMEM;
    page = alloc_page(gfp_flags);
    if (!page)
    {
        return NULL;
    }

    if (radix_tree_preload(GFP_NOIO))
    {
        __free_page(page);
        return NULL;
    }

    spin_lock(&ramDisk->Lock);
    idx = sector >> PAGE_SECTORS_SHIFT;
    page->index = idx;
    if (radix_tree_insert(&ramDisk->Pages, idx, page))
    {
        __free_page(page);
        page = radix_tree_lookup(&ramDisk->Pages, idx);
        BUG_ON(!page);
        BUG_ON(page->index != idx);
    }
    spin_unlock(&ramDisk->Lock);

    radix_tree_preload_end();

    return page;
}

/*
 * Free all backing store pages and radix tree. This must only be called when
 * there are no other users of the device.
 */
static void FreePages(struct blockRamDisk *ramDisk)
{
    unsigned long pos = 0;
    struct page *pages[FREE_BATCH];
    int nr_pages;

    do
    {
        int i;

        nr_pages = radix_tree_gang_lookup(&ramDisk->Pages,
                (void **)pages, pos, FREE_BATCH);

        for (i = 0; i < nr_pages; i++)
        {
            void *ret;

            BUG_ON(pages[i]->index < pos);
            pos = pages[i]->index;
            ret = radix_tree_delete(&ramDisk->Pages, pos);
            BUG_ON(!ret || ret != pages[i]);
            __free_page(pages[i]);
        }

        pos++;

        /*
         * It takes 3.4 seconds to remove 80GiB ramdisk.
         * So, we need cond_resched to avoid stalling the CPU.
         */
        cond_resched();

        /*
         * This assumes radix_tree_gang_lookup always returns as
         * many pages as possible. If the radix-tree code changes,
         * so will this have to.
         */
    } while (nr_pages == FREE_BATCH);
}

/*
 * CopyToRamDiskSetup must be called before CopyToRamDisk. It may sleep.
 */
static int CopyToRamDiskSetup(struct blockRamDisk *ramDisk, sector_t sector, size_t n)
{
    unsigned int offset = (sector & (PAGE_SECTORS-1)) << SECTOR_SHIFT;
    size_t copy;

    // pr_info("[CTRL][RAM] Disk[%d] :: Copy to RamDisk Setup\n", ramDisk->Number);

    copy = min_t(size_t, n, PAGE_SIZE - offset);
    if (!InsertPage(ramDisk, sector))
    {
        return -ENOSPC;
    }

    if (copy < n)
    {
        sector += copy >> SECTOR_SHIFT;
        if (!InsertPage(ramDisk, sector))
        {
            return -ENOSPC;
        }
    }

    return 0;
}

static void CopyToRamDisk(struct blockRamDisk *ramDisk, const void *src, sector_t sector, size_t n)
{
    struct page *page;
    void *dst;
    unsigned int offset = (sector & (PAGE_SECTORS - 1)) << SECTOR_SHIFT;
    size_t copy;

    // pr_info("[CTRL][RAM] Disk[%d] :: Copy to RamDisk\n", ramDisk->Number);

    copy = min_t(size_t, n, PAGE_SIZE - offset);
    page = LookupPage(ramDisk, sector);
    if (!page) {
        pr_err("Failed to lookup page for sector %llu\n", sector);
        return;
    }

    dst = kmap_atomic(page);
    memcpy(dst + offset, src, copy);
    kunmap_atomic(dst);

    if (copy < n) {
        src = (const char *)src + copy;
        sector += copy >> SECTOR_SHIFT;
        copy = n - copy;
        page = LookupPage(ramDisk, sector);
        if (!page) {
            pr_err("Failed to lookup page for next sector %llu\n", sector);
            return;
        }

        dst = kmap_atomic(page);
        memcpy(dst, src, copy);
        kunmap_atomic(dst);
    }
}

static void CopyFromRamDisk(void *dst, struct blockRamDisk *ramDisk, sector_t sector, size_t n)
{
    struct page *page;
    void *src;
    unsigned int offset = (sector & (PAGE_SECTORS-1)) << SECTOR_SHIFT;
    size_t copy;

    // pr_info("[CTRL][RAM] Disk[%d] :: Copy from RamDisk\n", ramDisk->Number);

    copy = min_t(size_t, n, PAGE_SIZE - offset);
    page = LookupPage(ramDisk, sector);
    if (page)
    {
        src = kmap_atomic(page);
        memcpy(dst, src + offset, copy);
        kunmap_atomic(src);
    }
    else
    {
        memset(dst, 0, copy);
    }

    if (copy < n)
    {
        dst += copy;
        sector += copy >> SECTOR_SHIFT;
        copy = n - copy;
        page = LookupPage(ramDisk, sector);
        if (page)
        {
            src = kmap_atomic(page);
            memcpy(dst, src, copy);
            kunmap_atomic(src);
        }
        else
        {
            memset(dst, 0, copy);
        }
    }
}

static int CheckReadOrWrite(struct blockRamDisk *ramDisk, struct page *page, unsigned int len, unsigned int off, unsigned int op, sector_t sector)
{
    void *mem;
    int err = 0;

    if (op_is_write(op))
    {
        err = CopyToRamDiskSetup(ramDisk, sector, len);
        if (err)
        {
            goto out;
        }
    }

    mem = kmap_atomic(page);
    if (!op_is_write(op))
    {
        CopyFromRamDisk(mem + off, ramDisk, sector, len);
        flush_dcache_page(page);
    }
    else
    {
        flush_dcache_page(page);
        CopyToRamDisk(ramDisk, mem + off, sector, len);
    }
    kunmap_atomic(mem);

out:
    return err;
}

static blk_qc_t BioRequest(struct bio *bio)
{
    struct blockRamDisk *ramDisk = bio->bi_disk->private_data;
    struct bio_vec bvec;
    sector_t sector;
    struct bvec_iter iter;
    // pr_info("[CTRL][RAM] Disk[%d] :: Block IO Request\n", ramDisk->Number);

    sector = bio->bi_iter.bi_sector;
    if (bio_end_sector(bio) > get_capacity(bio->bi_disk))
        goto io_error;

    bio_for_each_segment(bvec, bio, iter)
    {
        unsigned int len = bvec.bv_len;
        int err;

        /* Don't support un-aligned buffer */
        WARN_ON_ONCE((bvec.bv_offset & (SECTOR_SIZE - 1)) || (len & (SECTOR_SIZE - 1)));

        err = CheckReadOrWrite(ramDisk, bvec.bv_page, len, bvec.bv_offset, bio_op(bio), sector);

        if (err)
        {
            goto io_error;
        }

        sector += len >> SECTOR_SHIFT;
    }

    bio_endio(bio);
    return BLK_QC_T_NONE;

io_error:
    bio_io_error(bio);
    return BLK_QC_T_NONE;
}

static int RwPage(struct block_device *bdev, sector_t sector, struct page *page, unsigned int op)
{
    struct blockRamDisk *ramDisk = bdev->bd_disk->private_data;
    int err;

    if (PageTransHuge(page))
    {
        return -ENOTSUPP;
    }

    err = CheckReadOrWrite(ramDisk, page, PAGE_SIZE, 0, op, sector);
    page_endio(page, op_is_write(op), err);

    return err;
}

static struct blockRamDisk *DiskAllocation(int i)
{
    struct blockRamDisk *ramDisk;
    struct gendisk *disk;

    // pr_info("[INIT][RAM] Disk[%d] :: Memory Allocation\n", i);

    ramDisk = kzalloc(sizeof(*ramDisk), GFP_KERNEL);
    if (!ramDisk)
    {
        goto out;
    }

    ramDisk->Number     = i;
    spin_lock_init(&ramDisk->Lock);
    INIT_RADIX_TREE(&ramDisk->Pages, GFP_ATOMIC);

    ramDisk->Queue = blk_alloc_queue(NUMA_NO_NODE);
    if (!ramDisk->Queue)
    {
        goto out_free_dev;
    }

    /* This is so fdisk will align partitions on 4k, because of
     * direct_access API needing 4k alignment, returning a PFN
     * (This is only a problem on very small devices <= 4M,
     *  otherwise fdisk will align on 1M. Regardless this call
     *  is harmless)
     */
    blk_queue_physical_block_size(ramDisk->Queue, PAGE_SIZE);
    disk = ramDisk->Disk = alloc_disk(max_part);
    if (!disk)
    {
        goto out_free_queue;
    }

    disk->major = RAMDISK_MAJOR;
    disk->first_minor   = i * max_part;
    disk->fops = &fops;
    disk->private_data  = ramDisk;
    disk->flags = GENHD_FL_EXT_DEVT;
    sprintf(disk->disk_name, "IceNETDisk%d", i);
    set_capacity(disk, RAM_DISK_SIZE * 2);

    /* Tell the block layer that this is not a rotational device */
    blk_queue_flag_set(QUEUE_FLAG_NONROT, ramDisk->Queue);
    blk_queue_flag_clear(QUEUE_FLAG_ADD_RANDOM, ramDisk->Queue);

    return ramDisk;

out_free_queue:
    blk_cleanup_queue(ramDisk->Queue);
out_free_dev:
    kfree(ramDisk);
out:
    return NULL;
}

static void Free(struct blockRamDisk *ramDisk)
{
    put_disk(ramDisk->Disk);
    blk_cleanup_queue(ramDisk->Queue);
    FreePages(ramDisk);
    kfree(ramDisk);
}

static struct blockRamDisk *DiskAdd(int i, bool *new)
{
    struct blockRamDisk *ramDisk;

    // pr_info("[INIT][RAM] Disk[%d] :: Add\n", i);

    *new = false;
    list_for_each_entry(ramDisk, &iceDevices, List)
    {
        if (ramDisk->Number == i)
        {
            goto out;
        }
    }

    ramDisk = DiskAllocation(i);
    if (ramDisk)
    {
        ramDisk->Disk->queue = ramDisk->Queue;
        add_disk(ramDisk->Disk);
        list_add_tail(&ramDisk->List, &iceDevices);
    }
    *new = true;

out:
    return ramDisk;
}

static void DiskRemove(struct blockRamDisk *ramDisk)
{
    // pr_info("[DESTROY][RAM] Disk[%d] :: Remove\n", ramDisk->Number);

    list_del(&ramDisk->List);
    del_gendisk(ramDisk->Disk);
    Free(ramDisk);
}

static struct kobject *Probe(dev_t dev, int *part, void *data)
{
    struct blockRamDisk *ramDisk;
    struct kobject *kobj;
    bool new;

    // pr_info("[CTRL][RAM] Probe Detected\n");

    mutex_lock(&brd_devices_mutex);
    ramDisk = DiskAdd(MINOR(dev) / max_part, &new);
    kobj = ramDisk ? get_disk_and_module(ramDisk->Disk) : NULL;
    mutex_unlock(&brd_devices_mutex);

    if (new)
    {
        *part = 0;
    }

    return kobj;
}

static inline void CheckAndResetPartition(void)
{
    // pr_info("[INIT][RAM] Check and reset Disk Partition\n");

    if (unlikely(!max_part))
    {
        max_part = 1;
    }

    /*
     * make sure 'max_part' can be divided exactly by (1U << MINORBITS),
     * otherwise, it is possiable to get same dev_t when adding partitions.
     */
    if ((1U << MINORBITS) % max_part != 0)
    {
        max_part = 1UL << fls(max_part);
    }

    if (max_part > DISK_MAX_PARTS)
    {
        // pr_info("[ERROR][RAM] max_part can't be larger than %d, reset max_part = %d.\n", DISK_MAX_PARTS, DISK_MAX_PARTS);
        max_part = DISK_MAX_PARTS;
    }
}

void *ramDiskGetPointer(sector_t sector)
{
    struct block_device *bdev;
    struct page *page;
    void *page_data;
    int ret;

    bdev = blkdev_get_by_path("/dev/IceNETDisk0", FMODE_READ, NULL);
    if (IS_ERR(bdev)) 
    {
        pr_err("Failed to open IceNETDisk0\n");
        return NULL;
    }

    page = alloc_page(GFP_KERNEL);
    if (!page) 
    {
        blkdev_put(bdev, FMODE_READ);
        return NULL;
    }

    ret = RwPage(bdev, sector, page, REQ_OP_READ);
    if (ret) 
    {
        pr_err("Failed to read data from sector %llu: %d\n", sector, ret);
        __free_page(page);
        blkdev_put(bdev, FMODE_READ);
        return NULL;
    }

    page_data = kmap_atomic(page);
    // Keep the page mapped; return the pointer for direct access.
    blkdev_put(bdev, FMODE_READ);
    return page_data;
}

void ramDiskReleasePointer(void *page_data)
{
    // Unmap and free the page when done.
    if (page_data) 
    {
        kunmap_atomic(page_data);
    }
}

int ramDiskInit(void)
{
    struct blockRamDisk *ramDisk, *ramDiskNext;
    int i;

    // pr_info("[INIT][RAM] Initialization of Block ramDisk\n");

    /*
     * brd module now has a feature to instantiate underlying device
     * structure on-demand, provided that there is an access dev node.
     *
     * (1) if KERNEL_RAM_DISK_AMOUNT is specified, create that many upfront. else
     *     it defaults to CONFIG_BLK_DEV_RAM_COUNT
     * (2) User can further extend ramDisk devices by create dev node themselves
     *     and have kernel automatically instantiate actual device
     *     on-demand. Example:
     *      mknod /path/devnod_name b 1 X   # 1 is the rd major
     *      fdisk -l /path/devnod_name
     *  If (X / max_part) was not already created it will be created
     *  dynamically.
     */

    if (register_blkdev(RAMDISK_MAJOR, KERNEL_BLOCK_DEVICE))
        return -EIO;

    CheckAndResetPartition();

    for (i = 0; i < KERNEL_RAM_DISK_AMOUNT; i++)
    {
        ramDisk = DiskAllocation(i);
        if (!ramDisk)
        {
            goto out_free;
        }
        list_add_tail(&ramDisk->List, &iceDevices);
    }

    /* point of no return */

    list_for_each_entry(ramDisk, &iceDevices, List)
    {
        /*
         * associate with queue just before adding disk for
         * avoiding to mess up failure path
         */
        ramDisk->Disk->queue = ramDisk->Queue;
        add_disk(ramDisk->Disk);
    }

    blk_register_region(MKDEV(RAMDISK_MAJOR, 0), 1UL << MINORBITS, THIS_MODULE, Probe, NULL, NULL);

    pr_info("[INIT][RAM] Block ramDisk Initialised\n");

    return 0;

out_free:
    list_for_each_entry_safe(ramDisk, ramDiskNext, &iceDevices, List)
    {
        list_del(&ramDisk->List);
        Free(ramDisk);
    }
    unregister_blkdev(RAMDISK_MAJOR, KERNEL_BLOCK_DEVICE);

    // pr_info("[ERROR][RAM] Module NOT Loaded !!!\n");

    return -ENOMEM;
}

void ramDiskDestroy(void)
{
    struct blockRamDisk *ramDisk, *ramDiskNext;

    list_for_each_entry_safe(ramDisk, ramDiskNext, &iceDevices, List)
    {
        DiskRemove(ramDisk);
    }

    blk_unregister_region(MKDEV(RAMDISK_MAJOR, 0), 1UL << MINORBITS);
    unregister_blkdev(RAMDISK_MAJOR, KERNEL_BLOCK_DEVICE);

    pr_info("[DESTROY][RAM] Block ramDisk Destroyed\n");
}
