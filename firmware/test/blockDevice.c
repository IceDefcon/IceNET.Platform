#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>
#include <linux/slab.h>
#include <linux/hdreg.h>

#define DEVICE_NAME "iceBlock"
#define DEVICE_SIZE (1024 * 1024) // 1MB
#define MY_BLOCK_SIZE 512 // 512 bytes per block

static struct gendisk *my_disk;
static struct request_queue *queue;
static u8 *device_memory;

static int block_device_open(struct block_device *bdev, fmode_t mode) {
    printk(KERN_INFO "Block device opened\n");
    return 0;
}

static void block_device_release(struct gendisk *disk, fmode_t mode) {
    printk(KERN_INFO "Block device released\n");
}

static int block_device_getgeo(struct block_device *bdev, struct hd_geometry *geo) {
    geo->heads = 1;
    geo->sectors = DEVICE_SIZE / (geo->heads * MY_BLOCK_SIZE);
    geo->cylinders = DEVICE_SIZE / (geo->heads * geo->sectors * MY_BLOCK_SIZE);
    return 0;
}

static struct block_device_operations bdo = {
    .owner = THIS_MODULE,
    .open = block_device_open,
    .release = block_device_release,
    .getgeo = block_device_getgeo,
};

static int __init block_device_init(void) {
    // Allocate device memory
    device_memory = kmalloc(DEVICE_SIZE, GFP_KERNEL);
    if (!device_memory) {
        printk(KERN_ERR "Failed to allocate device memory\n");
        return -ENOMEM;
    }
    memset(device_memory, 0, DEVICE_SIZE);

    // Initialize request queue
    queue = blk_alloc_queue(GFP_KERNEL);
    if (!queue) {
        kfree(device_memory);
        printk(KERN_ERR "Failed to initialize queue\n");
        return -ENOMEM;
    }

    // Allocate gendisk structure
    my_disk = alloc_disk(1);
    if (!my_disk) {
        blk_cleanup_queue(queue);
        kfree(device_memory);
        printk(KERN_ERR "Failed to allocate disk\n");
        return -ENOMEM;
    }

    // Set up gendisk structure
    my_disk->major = register_blkdev(0, DEVICE_NAME);
    if (my_disk->major < 0) {
        del_gendisk(my_disk);
        blk_cleanup_queue(queue);
        kfree(device_memory);
        printk(KERN_ERR "Failed to register block device\n");
        return -EINVAL;
    }

    my_disk->first_minor = 0;
    my_disk->fops = &bdo;
    my_disk->queue = queue;
    sprintf(my_disk->disk_name, DEVICE_NAME);
    set_capacity(my_disk, DEVICE_SIZE / MY_BLOCK_SIZE);

    add_disk(my_disk);

    printk(KERN_INFO "Block device initialized\n");
    return 0;
}

static void __exit block_device_exit(void) {
    del_gendisk(my_disk);
    put_disk(my_disk);
    unregister_blkdev(my_disk->major, DEVICE_NAME);
    blk_cleanup_queue(queue);
    kfree(device_memory);
    printk(KERN_INFO "Block device exited\n");
}

module_init(block_device_init);
module_exit(block_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple Block Device Driver");
