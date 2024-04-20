#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/mutex.h>    // Include for mutex opearations

#define DEVICE_NAME "iceBLOCK"
#define DEVICE_SIZE (1024 * 1024) // 1MB
#define KERNEL_SECTOR_SIZE 512

static DEFINE_MUTEX(com_mutex);

static struct my_block_device {
    unsigned char *data;
    struct request_queue *queue;
    struct gendisk *gd;
} my_dev;

static int my_open(struct block_device *bdev, fmode_t mode) {
    return 0;
}

static void my_release(struct gendisk *disk, fmode_t mode) {
    return;
}

static struct block_device_operations my_ops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
};

static int __init my_block_device_init(void) {
    my_dev.data = vmalloc(DEVICE_SIZE);
    if (!my_dev.data)
        return -ENOMEM;

    my_dev.queue = blk_alloc_queue(GFP_KERNEL);
    if (!my_dev.queue)
        goto out;

    blk_queue_logical_block_size(my_dev.queue, KERNEL_SECTOR_SIZE);

    my_dev.gd = alloc_disk(1);
    if (!my_dev.gd)
        goto out;

    my_dev.gd->major = register_blkdev(0, DEVICE_NAME);
    if (my_dev.gd->major < 0) {
        printk(KERN_ERR "Failed to register block device with error: %d\n", my_dev.gd->major);
        goto out_unregister;
    }

    printk(KERN_INFO "Registered block device with major number: %d\n", my_dev.gd->major);

    my_dev.gd->queue = my_dev.queue;
    my_dev.gd->private_data = &my_dev;
    strcpy(my_dev.gd->disk_name, DEVICE_NAME);
    set_capacity(my_dev.gd, DEVICE_SIZE / KERNEL_SECTOR_SIZE);

    my_dev.gd->fops = &my_ops;
    add_disk(my_dev.gd);

    mutex_init(&com_mutex);
    printk(KERN_INFO "Block device registered\n");
    return 0;

out_unregister:
    unregister_blkdev(my_dev.gd->major, DEVICE_NAME);
out:
    vfree(my_dev.data);
    return -ENOMEM;
}

static void my_block_device_exit(void) {
    del_gendisk(my_dev.gd);
    put_disk(my_dev.gd);
    blk_cleanup_queue(my_dev.queue);
    vfree(my_dev.data);

    printk(KERN_INFO "Unregistering block device with major number: %d\n", my_dev.gd->major);

    unregister_blkdev(my_dev.gd->major, DEVICE_NAME);
    mutex_destroy(&com_mutex);
    printk(KERN_INFO "Block device unregistered\n");
}

module_init(my_block_device_init);
module_exit(my_block_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple block device module");
