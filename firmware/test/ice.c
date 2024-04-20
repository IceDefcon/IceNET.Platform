#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>
#include <linux/slab.h>
#include <linux/hdreg.h>
#include <linux/stat.h> // Include this header for S_ISBLK macro

#define KERNEL_SECTOR_SIZE 512
#define DEVICE_SIZE 1024 * 1024

static struct block_device_operations iceBlock_ops = {
    .owner = THIS_MODULE,
};

static struct block_device *iceBlock_bdev;

static int __init iceBlock_init(void)
{
    int ret;

    // Allocate memory for the block device
    iceBlock_bdev = blkdev_get_by_path("/dev/mmcblk0p1", FMODE_READ | FMODE_WRITE, NULL);

    if (IS_ERR(iceBlock_bdev)) {
        printk(KERN_ERR "Failed to get block device\n");
        return PTR_ERR(iceBlock_bdev);
    }

    // Set up the block device
    iceBlock_bdev->bd_inode->i_size = DEVICE_SIZE;
    iceBlock_bdev->bd_inode->i_blkbits = 9; // block size = 512 bytes (2^9)
    iceBlock_bdev->bd_inode->i_flags |= S_ISBLK;
    iceBlock_bdev->bd_inode->i_op = &iceBlock_bdev->bd_blkdev_ops;

    // Register the block device
    ret = register_blkdev(0, "iceBlock");
    if (ret < 0) {
        printk(KERN_ERR "Failed to register block device: %d\n", ret);
        return ret;
    }

    return 0;
}

static void __exit iceBlock_exit(void)
{
    // Unregister the block device
    unregister_blkdev(MAJOR(iceBlock_bdev->bd_dev), "iceBlock");
    blkdev_put(iceBlock_bdev, FMODE_READ | FMODE_WRITE);
}

module_init(iceBlock_init);
module_exit(iceBlock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple virtual block device module named iceBlock");
