#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/mutex.h>    // Include for mutex opearations

#define DEVICE_NAME "iceBLOCK"
#define DEVICE_SIZE (1024 * 1024) // 1MB
#define KERNEL_SECTOR_SIZE 512

static DEFINE_MUTEX(com_mutex);

static struct iceBlockDevice {
    unsigned char *data;
    struct request_queue *queue;
    struct gendisk *gd;
} iceBlock;

static int dev_open(struct block_device *bdev, fmode_t mode) {
    return 0;
}

static void dev_release(struct gendisk *disk, fmode_t mode) {
    return;
}

static struct block_device_operations my_ops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
};

static int __init block_device_init(void) {
    iceBlock.data = vmalloc(DEVICE_SIZE);
    if (!iceBlock.data)
        return -ENOMEM;

    iceBlock.queue = blk_alloc_queue(GFP_KERNEL);
    if (!iceBlock.queue)
    {
        vfree(iceBlock.data);
        return -ENOMEM;
    }

    blk_queue_logical_block_size(iceBlock.queue, KERNEL_SECTOR_SIZE);

    iceBlock.gd = alloc_disk(1);
    if (!iceBlock.gd)
    {
        vfree(iceBlock.data);
        return -ENOMEM;
    }

    iceBlock.gd->major = register_blkdev(0, DEVICE_NAME);
    if (iceBlock.gd->major < 0) {
        printk(KERN_ERR "Failed to register block device with error: %d\n", iceBlock.gd->major);
        unregister_blkdev(iceBlock.gd->major, DEVICE_NAME);
    }

    printk(KERN_INFO "Registered block device with major number: %d\n", iceBlock.gd->major);

    iceBlock.gd->queue = iceBlock.queue;
    iceBlock.gd->private_data = &iceBlock;
    strcpy(iceBlock.gd->disk_name, DEVICE_NAME);
    set_capacity(iceBlock.gd, DEVICE_SIZE / KERNEL_SECTOR_SIZE);

    iceBlock.gd->fops = &my_ops;
    add_disk(iceBlock.gd);

    mutex_init(&com_mutex);
    printk(KERN_INFO "Block device registered\n");
    return 0;
}

static void __exit block_device_exit(void)
{
    printk(KERN_INFO "Exiting block_device_exit\n");

    if (iceBlock.gd) {
        printk(KERN_INFO "Deleting gendisk with major number %d\n", iceBlock.gd->major);
        del_gendisk(iceBlock.gd);
        printk(KERN_INFO "Gendisk deleted >> checking major number %d\n", iceBlock.gd->major);
    } else {
        printk(KERN_WARNING "Gendisk does not exist\n");
    }

    if (iceBlock.queue) {
        printk(KERN_INFO "Cleaning up queue >> checking major number %d\n", iceBlock.gd->major);
        blk_cleanup_queue(iceBlock.queue);
        printk(KERN_INFO "Queue cleaned up >> checking major number %d\n", iceBlock.gd->major);
    } else {
        printk(KERN_WARNING "Queue does not exist\n");
    }

    if (iceBlock.gd) {
        printk(KERN_INFO "Unregistering block device with major number %d\n", iceBlock.gd->major);
        unregister_blkdev(iceBlock.gd->major, DEVICE_NAME);
        printk(KERN_INFO "Block device unregistered >> checking major number %d\n", iceBlock.gd->major);
    } else {
        printk(KERN_WARNING "Gendisk does not exist for unregistering\n");
    }

    if (iceBlock.gd) {
        printk(KERN_INFO "Putting gendisk >> checking major number %d\n", iceBlock.gd->major);
        put_disk(iceBlock.gd);
        printk(KERN_INFO "Disk put >> checking major number %d\n", iceBlock.gd->major);
    } else {
        printk(KERN_WARNING "Gendisk does not exist for putting\n");
    }

    if (iceBlock.data) {
        printk(KERN_INFO "Freeing data >> checking major number %d\n", iceBlock.gd->major);
        vfree(iceBlock.data);
        printk(KERN_INFO "Data freed >> checking major number %d\n", iceBlock.gd->major);
    } else {
        printk(KERN_WARNING "Data does not exist\n");
    }

    mutex_destroy(&com_mutex);
    printk(KERN_INFO "Block device exit completed >> checking major number %d\n", iceBlock.gd->major);
}



module_init(block_device_init);
module_exit(block_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple block device module");
