#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/blk-mq.h>
#include <linux/slab.h> // For kmalloc and kfree

#define DEVICE_NAME "KernelBlock"
#define DEVICE_SIZE (1024 * 1024) // 1MB device size
#define KERNELBLOCK_SECTOR_SIZE 512 // Custom sector size

struct kernel_block_device {
    int major_num;                     // Major number
    struct gendisk *gd;                // General disk structure
    struct blk_mq_tag_set tag_set;     // Multi-queue tag set
    struct request_queue *queue;       // Request queue
    char *data;                        // Device data buffer
};

static struct kernel_block_device kblock_dev;

// Forward declarations
static blk_status_t kernelBlock_request(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data *bd);
static int kernelBlock_open(struct block_device *bdev, fmode_t mode);
static void kernelBlock_release(struct gendisk *gd, fmode_t mode);

// Block device operations
static const struct block_device_operations kernelBlock_ops = {
    .owner = THIS_MODULE,
    .open = kernelBlock_open,
    .release = kernelBlock_release,
};

// Multi-queue operations
static const struct blk_mq_ops kernelBlock_mq_ops = {
    .queue_rq = kernelBlock_request,
};

// Request handling function
static blk_status_t kernelBlock_request(struct blk_mq_hw_ctx *hctx,
                                        const struct blk_mq_queue_data *bd)
{
    struct bio *bio = bd->rq->bio;
    unsigned long offset;
    unsigned int i;
    unsigned int size;

    offset = bio->bi_iter.bi_sector * KERNELBLOCK_SECTOR_SIZE;
    size = bio->bi_iter.bi_size;

    if (offset + size > DEVICE_SIZE) {
        printk(KERN_ERR "kernelBlock_driver: I/O beyond device size\n");
        bio->bi_status = BLK_STS_IOERR;
        bio_endio(bio);
        return BLK_STS_IOERR;
    }

    if (bio_data_dir(bio) == WRITE) {
        for (i = 0; i < size; i++) {
            kblock_dev.data[offset + i] = ((char *)bio_data(bio))[i];
        }
    } else {
        for (i = 0; i < size; i++) {
            ((char *)bio_data(bio))[i] = kblock_dev.data[offset + i];
        }
    }

    bio_endio(bio);
    return BLK_STS_OK;
}

// Open operation
static int kernelBlock_open(struct block_device *bdev, fmode_t mode)
{
    printk(KERN_INFO "kernelBlock_driver: Device opened\n");
    return 0;
}

// Release operation
static void kernelBlock_release(struct gendisk *gd, fmode_t mode)
{
    printk(KERN_INFO "kernelBlock_driver: Device released\n");
}

// Initialization function
static int __init kernelBlock_init(void)
{
    int ret;

    // Allocate device data
    kblock_dev.data = kmalloc(DEVICE_SIZE, GFP_KERNEL);
    if (!kblock_dev.data) {
        printk(KERN_ERR "kernelBlock_driver: Memory allocation failed\n");
        return -ENOMEM;
    }
    memset(kblock_dev.data, 0, DEVICE_SIZE);

    // Register block device
    ret = register_blkdev(0, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "kernelBlock_driver: Failed to register block device\n");
        kfree(kblock_dev.data);
        return ret;
    }
    kblock_dev.major_num = ret;

    // Initialize tag set
    memset(&kblock_dev.tag_set, 0, sizeof(kblock_dev.tag_set));
    kblock_dev.tag_set.ops = &kernelBlock_mq_ops;
    kblock_dev.tag_set.queue_depth = 64;
    kblock_dev.tag_set.numa_node = NUMA_NO_NODE;
    kblock_dev.tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
    kblock_dev.tag_set.nr_hw_queues = 1;

    ret = blk_mq_alloc_tag_set(&kblock_dev.tag_set);
    if (ret) {
        printk(KERN_ERR "kernelBlock_driver: Failed to allocate tag set\n");
        unregister_blkdev(kblock_dev.major_num, DEVICE_NAME);
        kfree(kblock_dev.data);
        return ret;
    }

    kblock_dev.queue = blk_mq_init_queue(&kblock_dev.tag_set);
    if (IS_ERR(kblock_dev.queue)) {
        ret = PTR_ERR(kblock_dev.queue);
        printk(KERN_ERR "kernelBlock_driver: Failed to initialize request queue\n");
        blk_mq_free_tag_set(&kblock_dev.tag_set);
        unregister_blkdev(kblock_dev.major_num, DEVICE_NAME);
        kfree(kblock_dev.data);
        return ret;
    }

    // Allocate and configure gendisk
    kblock_dev.gd = alloc_disk(1);
    if (!kblock_dev.gd) {
        blk_cleanup_queue(kblock_dev.queue);
        blk_mq_free_tag_set(&kblock_dev.tag_set);
        unregister_blkdev(kblock_dev.major_num, DEVICE_NAME);
        kfree(kblock_dev.data);
        return -ENOMEM;
    }

    kblock_dev.gd->major = kblock_dev.major_num;
    kblock_dev.gd->first_minor = 0;
    kblock_dev.gd->fops = &kernelBlock_ops;
    kblock_dev.gd->queue = kblock_dev.queue;
    snprintf(kblock_dev.gd->disk_name, 32, DEVICE_NAME);
    set_capacity(kblock_dev.gd, DEVICE_SIZE / KERNELBLOCK_SECTOR_SIZE);
    add_disk(kblock_dev.gd);

    printk(KERN_INFO "kernelBlock_driver: Device initialized successfully\n");
    return 0;
}

// Cleanup function
static void __exit kernelBlock_exit(void)
{
    if (kblock_dev.gd) {
        del_gendisk(kblock_dev.gd);
        put_disk(kblock_dev.gd);
    }

    if (kblock_dev.queue) {
        blk_cleanup_queue(kblock_dev.queue);
    }

    blk_mq_free_tag_set(&kblock_dev.tag_set);
    unregister_blkdev(kblock_dev.major_num, DEVICE_NAME);

    if (kblock_dev.data) {
        kfree(kblock_dev.data);
    }

    printk(KERN_INFO "kernelBlock_driver: Device unregistered successfully\n");
}

module_init(kernelBlock_init);
module_exit(kernelBlock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice Marek");
MODULE_DESCRIPTION("IceNET Block Device");
