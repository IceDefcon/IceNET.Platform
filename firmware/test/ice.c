#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>

#define ICEBLOCK_MAJOR 240
#define ICEBLOCK_MINOR 0
#define ICEBLOCK_SIZE 1024 * 1024 // 1MB size

static struct block_device_operations iceblock_ops = {
    .owner = THIS_MODULE,
};

static struct block_device iceblock_device = {
    .bd_disk = {
        .major = ICEBLOCK_MAJOR,
        .first_minor = ICEBLOCK_MINOR,
        .fops = &iceblock_ops,
        .private_data = NULL,
        .queue = NULL,
    },
    .bd_size = ICEBLOCK_SIZE,
    .bd_block_size = 512,
    .bd_disk = &iceblock_device.bd_disk,
    .bd_invalidated = 1,
};

static int __init iceblock_init(void)
{
    int ret;

    ret = register_blkdev(ICEBLOCK_MAJOR, "iceBlock");
    if (ret < 0) {
        printk(KERN_ERR "iceblock: Failed to register block device\n");
        return ret;
    }

    iceblock_device.bd_disk->major = ICEBLOCK_MAJOR;
    iceblock_device.bd_disk->first_minor = ICEBLOCK_MINOR;
    iceblock_device.bd_disk->fops = &iceblock_ops;

    ret = add_disk(&iceblock_device);
    if (!ret) {
        printk(KERN_INFO "iceblock: Block device registered successfully\n");
    } else {
        printk(KERN_ERR "iceblock: Failed to add block device\n");
        unregister_blkdev(ICEBLOCK_MAJOR, "iceBlock");
    }

    return ret;
}

static void __exit iceblock_exit(void)
{
    del_gendisk(&iceblock_device);
    put_disk(&iceblock_device);
    unregister_blkdev(ICEBLOCK_MAJOR, "iceBlock");
    printk(KERN_INFO "iceblock: Unregistered block device\n");
}

module_init(iceblock_init);
module_exit(iceblock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple block device driver for /dev/iceBlock");
