#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/uaccess.h>
#include <linux/bio.h>
#include <linux/blk-mq.h>

#define DEVICE_NAME "KernelBlock"
static int major_num = 0;  // Leave major number as 0 to request dynamically

static struct gendisk *gendisk;
static char *device_data; // Data buffer for block device
static int device_size = 1024; // Size of the block device (1MB)
static int sector_size = 512;  // Sector size in bytes

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

// Define blk_mq operations
static const struct blk_mq_ops kernelBlock_mq_ops = {
    .queue_rq = kernelBlock_request,  // The function to handle I/O requests
};

// The read and write operations should use bio
static blk_status_t kernelBlock_request(struct blk_mq_hw_ctx *hctx,
                                       const struct blk_mq_queue_data *bd)
{
    struct bio *bio = bd->rq->bio;
    unsigned long offset;
    unsigned int i;
    unsigned int size;

    offset = bio->bi_iter.bi_sector * sector_size;
    size = bio->bi_iter.bi_size;

    if (offset + size > device_size) {
        printk(KERN_ERR "kernelBlock_driver: I/O beyond device size\n");
        bio->bi_status = BLK_STS_IOERR;
        bio_endio(bio);
        return BLK_STS_IOERR;
    }

    if (bio_data_dir(bio) == WRITE) {
        for (i = 0; i < size; i++) {
            device_data[offset + i] = ((char *)bio_data(bio))[i];
        }
    } else {
        for (i = 0; i < size; i++) {
            ((char *)bio_data(bio))[i] = device_data[offset + i];
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
    struct blk_mq_tag_set tag_set = {
        .ops = &kernelBlock_mq_ops,
        .queue_depth = 64,
        .numa_node = NUMA_NO_NODE,
        .flags = BLK_MQ_F_SHOULD_MERGE,
        .driver_data = gendisk,
        .nr_hw_queues = 1,
    };

    device_data = kmalloc(device_size, GFP_KERNEL);
    if (!device_data) {
        printk(KERN_ERR "kernelBlock_driver: Memory allocation failed\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "kernelBlock_driver: Memory allocated for device data\n");

    ret = register_blkdev(major_num, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "kernelBlock_driver: Failed to register block device\n");
        kfree(device_data);
        return ret;
    }

    major_num = ret;
    printk(KERN_INFO "kernelBlock_driver: Major number assigned: %d\n", major_num);

    gendisk = alloc_disk(1);
    if (!gendisk) {
        unregister_blkdev(major_num, DEVICE_NAME);
        kfree(device_data);
        return -ENOMEM;
    }

    gendisk->major = major_num;
    gendisk->first_minor = 0;
    gendisk->fops = &kernelBlock_ops;
    snprintf(gendisk->disk_name, 32, DEVICE_NAME);

    ret = blk_mq_alloc_tag_set(&tag_set);
    if (ret) {
        printk(KERN_ERR "kernelBlock_driver: Failed to allocate tag set\n");
        kfree(device_data);
        unregister_blkdev(major_num, DEVICE_NAME);
        put_disk(gendisk);
        return ret;
    }

    gendisk->queue = blk_mq_init_queue(&tag_set);
    if (IS_ERR(gendisk->queue)) {
        ret = PTR_ERR(gendisk->queue);
        printk(KERN_ERR "kernelBlock_driver: Failed to initialize request queue, error: %ld\n", ret);
        blk_mq_free_tag_set(&tag_set);
        kfree(device_data);
        unregister_blkdev(major_num, DEVICE_NAME);
        put_disk(gendisk);
        return ret;
    }

    set_capacity(gendisk, device_size / sector_size);

    add_disk(gendisk);
    printk(KERN_INFO "kernelBlock_driver: Block device registered successfully\n");

    return 0;
}

static void __exit kernelBlock_exit(void)
{
    printk(KERN_INFO "kernelBlock_driver: Cleanup starting...\n");

    printk(KERN_INFO "kernelBlock_driver: Debug 00\n");
    // Free memory for the tag set if allocated
    if (gendisk && gendisk->queue && gendisk->queue->tag_set) {
        printk(KERN_INFO "kernelBlock_driver: Debug 01\n");
        blk_mq_free_tag_set(gendisk->queue->tag_set);
    }

    printk(KERN_INFO "kernelBlock_driver: Debug 02\n");
    // Clean up request queue
    if (gendisk && gendisk->queue) {
        printk(KERN_INFO "kernelBlock_driver: Debug 03\n");
        blk_cleanup_queue(gendisk->queue);
        printk(KERN_INFO "kernelBlock_driver: Debug 04\n");
        gendisk->queue = NULL; // Prevent double cleanup
    }

    // Ensure no further I/O operations are attempted
    if (gendisk) {
        printk(KERN_INFO "kernelBlock_driver: Debug 05\n");
        del_gendisk(gendisk);
        printk(KERN_INFO "kernelBlock_driver: Debug 06\n");
        put_disk(gendisk); // Decrement reference count
        printk(KERN_INFO "kernelBlock_driver: Debug 07\n");
        gendisk = NULL;    // Mark as cleaned
    }

    printk(KERN_INFO "kernelBlock_driver: Debug 08\n");
    // Unregister the block device
    unregister_blkdev(major_num, DEVICE_NAME);

    printk(KERN_INFO "kernelBlock_driver: Debug 09\n");
    // Free the allocated memory for device data
    if (device_data) {
        printk(KERN_INFO "kernelBlock_driver: Debug 10\n");
        kfree(device_data);
        printk(KERN_INFO "kernelBlock_driver: Debug 11\n");
        device_data = NULL;
    }

    printk(KERN_INFO "kernelBlock_driver: Block device unregistered successfully\n");
}




module_init(kernelBlock_init);
module_exit(kernelBlock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice Marek");
MODULE_DESCRIPTION("IceNET Block Device");
