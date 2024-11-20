#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/blk-mq.h>
#include <linux/slab.h> // For kmalloc and kfree
#include <linux/delay.h> // For msleep

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

static blk_status_t kernelBlock_request(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data *bd);
static int kernelBlock_open(struct block_device *bdev, fmode_t mode);
static void kernelBlock_release(struct gendisk *gd, fmode_t mode);

static const struct block_device_operations kernelBlock_ops =
{
    .owner = THIS_MODULE,
    .open = kernelBlock_open,
    .release = kernelBlock_release,
};

static const struct blk_mq_ops kernelBlock_mq_ops =
{
    .queue_rq = kernelBlock_request,
};

static blk_status_t kernelBlock_request(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data *bd)
{
    struct bio *bio = bd->rq->bio;
    unsigned long offset;
    unsigned int i;
    unsigned int size;
    char *data_ptr = (char *)bio_data(bio); // Pointer to the bio data

    printk(KERN_INFO "kernelBlock_driver: Received I/O request - bio: %p, sector: %llu, size: %u\n", bio, bio->bi_iter.bi_sector, bio->bi_iter.bi_size);

    offset = bio->bi_iter.bi_sector * KERNELBLOCK_SECTOR_SIZE;
    size = bio->bi_iter.bi_size;

    printk(KERN_INFO "kernelBlock_driver: Checking I/O bounds, offset: %lu, size: %u\n", offset, size);

    if (offset + size > DEVICE_SIZE) /* Check if the requested I/O operation is beyond the device size */
    {
        printk(KERN_ERR "kernelBlock_driver: I/O beyond device size\n");
        bio->bi_status = BLK_STS_IOERR;
        bio_endio(bio);
        return BLK_STS_IOERR;
    }

    if (bio_data_dir(bio) == WRITE)
    {
        printk(KERN_INFO "Writing to device at offset: %lu, size: %u\n", offset, size);
        printk(KERN_INFO "Data to write: ");
        for (i = 0; i < size; i++)
        {
            printk(KERN_CONT "%02X ", (unsigned char)data_ptr[i]);
            if (i == 63) break; /* Limit print size to 64 bytes */
        }
        printk(KERN_CONT "\n");

        memcpy(&kblock_dev.data[offset], data_ptr, size); /* Perform the write operation */
    }
    else if (bio_data_dir(bio) == READ)
    {
        printk(KERN_INFO "Reading from device at offset: %lu, size: %u\n", offset, size);
        printk(KERN_INFO "Data read: ");
        for (i = 0; i < size; i++)
        {
            printk(KERN_CONT "%02X ", (unsigned char)kblock_dev.data[offset + i]);
            if (i == 63) break; /* Limit print size to 64 bytes */
        }
        printk(KERN_CONT "\n");

        memcpy(data_ptr, &kblock_dev.data[offset], size); /* Perform the read operation */
    }
    else
    {
        printk(KERN_ERR "Unsupported bio_data_dir operation: %d\n", bio_data_dir(bio));
        return -EOPNOTSUPP; /* Operation not supported */
    }

    printk(KERN_INFO "kernelBlock_driver: Completing I/O request (bio: %p) at offset: %lu\n", bio, offset);

    bio_endio(bio); /* Ensure to end the request after completion (whether successful or error) */

    printk(KERN_INFO "kernelBlock_driver: I/O request completed with status: %d\n", BLK_STS_OK);

    return BLK_STS_OK;
}

static int kernelBlock_open(struct block_device *bdev, fmode_t mode)
{
    printk(KERN_INFO "kernelBlock_driver: Device opened\n");
    return 0;
}

static void kernelBlock_release(struct gendisk *gd, fmode_t mode)
{
    printk(KERN_INFO "kernelBlock_driver: Device released\n");
}

static int __init kernelBlock_init(void)
{
    int ret;

    printk(KERN_INFO "kernelBlock_driver: Initializing module\n");

    // Allocate device data
    printk(KERN_INFO "kernelBlock_driver: Allocating memory for device data\n");
    kblock_dev.data = kmalloc(DEVICE_SIZE, GFP_KERNEL);
    if (!kblock_dev.data)
    {
        printk(KERN_ERR "kernelBlock_driver: Memory allocation failed\n");
        return -ENOMEM;
    }
    memset(kblock_dev.data, 0, DEVICE_SIZE);
    printk(KERN_INFO "kernelBlock_driver: Memory allocated and initialized\n");

    // Register block device
    printk(KERN_INFO "kernelBlock_driver: Registering block device\n");
    ret = register_blkdev(0, DEVICE_NAME);
    if (ret < 0)
    {
        printk(KERN_ERR "kernelBlock_driver: Failed to register block device\n");
        kfree(kblock_dev.data);
        return ret;
    }
    kblock_dev.major_num = ret;
    printk(KERN_INFO "kernelBlock_driver: Block device registered with major number %d\n", kblock_dev.major_num);

    // Initialize tag set
    printk(KERN_INFO "kernelBlock_driver: Initializing tag set\n");
    memset(&kblock_dev.tag_set, 0, sizeof(kblock_dev.tag_set));
    kblock_dev.tag_set.ops = &kernelBlock_mq_ops;
    kblock_dev.tag_set.queue_depth = 64;
    kblock_dev.tag_set.numa_node = NUMA_NO_NODE;
    kblock_dev.tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
    kblock_dev.tag_set.nr_hw_queues = 1;

    ret = blk_mq_alloc_tag_set(&kblock_dev.tag_set);
    if (ret)
    {
        printk(KERN_ERR "kernelBlock_driver: Failed to allocate tag set\n");
        unregister_blkdev(kblock_dev.major_num, DEVICE_NAME);
        kfree(kblock_dev.data);
        return ret;
    }
    printk(KERN_INFO "kernelBlock_driver: Tag set allocated\n");

    kblock_dev.queue = blk_mq_init_queue(&kblock_dev.tag_set);
    if (IS_ERR(kblock_dev.queue))
    {
        ret = PTR_ERR(kblock_dev.queue);
        printk(KERN_ERR "kernelBlock_driver: Failed to initialize request queue\n");
        blk_mq_free_tag_set(&kblock_dev.tag_set);
        unregister_blkdev(kblock_dev.major_num, DEVICE_NAME);
        kfree(kblock_dev.data);
        return ret;
    }
    printk(KERN_INFO "kernelBlock_driver: Request queue initialized\n");

    // Allocate and configure gendisk
    printk(KERN_INFO "kernelBlock_driver: Allocating gendisk\n");
    kblock_dev.gd = alloc_disk(1);
    if (!kblock_dev.gd)
    {
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

static void __exit kernelBlock_exit(void)
{
    printk(KERN_INFO "kernelBlock_driver: Debug 0 - Exiting module\n");

    // Debug check before freezing the queue
    printk(KERN_INFO "kernelBlock_driver: Pending requests: %d\n", kblock_dev.queue->nr_pending);

    // Freeze the queue and wait for pending requests to finish
    printk(KERN_INFO "kernelBlock_driver: Quiescing the request queue\n");
    blk_mq_quiesce_queue(kblock_dev.queue);  // Quiesce the queue to prevent new requests
    printk(KERN_INFO "kernelBlock_driver: Debug 1 - Queue quiesced, Pending requests: %d\n", kblock_dev.queue->nr_pending);

    // Wait for all pending requests to finish before proceeding
    while (kblock_dev.queue->nr_pending > 0)
    {
        printk(KERN_INFO "kernelBlock_driver: Waiting for pending requests to finish... Pending: %d\n", kblock_dev.queue->nr_pending);

        // Wait for the requests to finish
        if (wait_event_timeout(kblock_dev.queue->mq_freeze_wq, percpu_ref_is_zero(&kblock_dev.queue->q_usage_counter), msecs_to_jiffies(5000)))
        {
            break;  // If the queue becomes idle, we break out of the loop
        }

        printk(KERN_WARNING "kernelBlock_driver: Timeout while waiting for pending requests\n");
    }

    // Check if the queue is now idle or if we timed out
    if (kblock_dev.queue->nr_pending > 0)
    {
        printk(KERN_WARNING "kernelBlock_driver: Still have pending requests, forcing cleanup...\n");
        // Handle the situation where there are still pending requests after the timeout
        // Consider canceling or aborting requests if needed, depending on your driver logic
    }

    // Debug info after waiting for I/O to finish
    printk(KERN_INFO "kernelBlock_driver: Debug 2 - Removing gendisk\n");

    if (kblock_dev.gd)
    {
        del_gendisk(kblock_dev.gd);
        printk(KERN_INFO "kernelBlock_driver: Debug 3 - gendisk removed\n");
        put_disk(kblock_dev.gd);
    }

    // Clean up the queue and free resources
    printk(KERN_INFO "kernelBlock_driver: Debug 4 - Cleaning up queue\n");
    blk_cleanup_queue(kblock_dev.queue);

    // Free the tag set and unregister the block device
    printk(KERN_INFO "kernelBlock_driver: Debug 5 - Freeing tag set\n");
    blk_mq_free_tag_set(&kblock_dev.tag_set);

    printk(KERN_INFO "kernelBlock_driver: Debug 6 - Unregistering block device\n");
    unregister_blkdev(kblock_dev.major_num, DEVICE_NAME);

    // Free the device data
    printk(KERN_INFO "kernelBlock_driver: Debug 7 - Freeing device data\n");
    if (kblock_dev.data)
    {
        kfree(kblock_dev.data);
    }

    printk(KERN_INFO "kernelBlock_driver: Debug 8 - Module unloaded successfully\n");
}

module_init(kernelBlock_init);
module_exit(kernelBlock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice Marek");
MODULE_DESCRIPTION("IceNET Block Device");
