#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/init.h>
#include <linux/types.h>

#define ICEBLOCK_MAJOR 240
#define ICEBLOCK_MINOR 0
#define ICEBLOCK_SIZE (1024 * 1024) // 1MB size

static struct gendisk *iceblock_disk;
static struct request_queue *iceblock_queue;

static int iceblock_open(struct block_device *bdev, fmode_t mode)
{
    return 0;
}

static void iceblock_release(struct gendisk *gd, fmode_t mode)
{
    return;
}

static struct block_device_operations iceblock_ops = {
    .owner = THIS_MODULE,
    .open = iceblock_open,
    .release = iceblock_release,
};

static int __init iceblock_init(void)
{
    iceblock_disk = alloc_disk(1);
    if (!iceblock_disk) {
        printk(KERN_ERR "iceblock: Failed to allocate gendisk structure\n");
        return -ENOMEM;
    }

    iceblock_queue = blk_init_queue(iceblock_queue, NULL);
    if (!iceblock_queue) {
        put_disk(iceblock_disk);
        printk(KERN_ERR "iceblock: Failed to initialize request queue\n");
        return -ENOMEM;
    }

    iceblock_disk->major = ICEBLOCK_MAJOR;
    iceblock_disk->first_minor = ICEBLOCK_MINOR;
    iceblock_disk->fops = &iceblock_ops;
    iceblock_disk->queue = iceblock_queue;
    sprintf(iceblock_disk->disk_name, "iceBlock");
    set_capacity(iceblock_disk, ICEBLOCK_SIZE / 512);

    add_disk(iceblock_disk);
    printk(KERN_INFO "iceblock: Block device registered successfully\n");

    return 0;
}

static void __exit iceblock_exit(void)
{
    del_gendisk(iceblock_disk);
    put_disk(iceblock_disk);
    blk_cleanup_queue(iceblock_queue);
    printk(KERN_INFO "iceblock: Unregistered block device\n");
}

module_init(iceblock_init);
module_exit(iceblock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple block device driver for /dev/iceBlock");
