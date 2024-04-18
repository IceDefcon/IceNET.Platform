#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>
#include <linux/slab.h>

#define DEVICE_NAME "my_block_device"
#define DEVICE_SIZE (1024 * 1024) // 1MB
#define BLOCK_SIZE 512 // 512 bytes per block

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

static int block_device_read(struct block_device *bdev, sector_t sector,
                             void *buffer, unsigned long size) {
    unsigned long offset = sector * BLOCK_SIZE;
    if (offset + size > DEVICE_SIZE) {
        printk(KERN_ERR "Read beyond end of device\n");
        return -EINVAL;
    }
    memcpy(buffer, device_memory + offset, size);
    return 0;
}

static int block_device_write(struct block_device *bdev, sector_t sector,
                              const void *buffer, unsigned long size) {
    unsigned long offset = sector * BLOCK_SIZE;
    if (offset + size > DEVICE_SIZE) {
        printk(KERN_ERR "Write beyond end of device\n");
        return -EINVAL;
    }
    memcpy(device_memory + offset, buffer, size);
    return 0;
}

static struct block_device_operations bdo = {
    .owner = THIS_MODULE,
    .open = block_device_open,
    .release = block_device_release,
    .read = block_device_read,
    .write = block_device_write,
};

static int __init block_device_init(void) {
    int ret;

    // Allocate device memory
    device_memory = kmalloc(DEVICE_SIZE, GFP_KERNEL);
    if (!device_memory) {
        printk(KERN_ERR "Failed to allocate device memory\n");
        return -ENOMEM;
    }
    memset(device_memory, 0, DEVICE_SIZE);

    // Initialize request queue
    queue = blk_init_queue(block_device_queue, NULL);
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
    set_capacity(my_disk, DEVICE_SIZE / BLOCK_SIZE);

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
