#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/blkdev.h>

#define DEVICE_NAME "iceBLOCK"
#define DEVICE_SIZE (1024 * 1024) // 1MB
#define KERNEL_SECTOR_SIZE 512

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

static int my_getgeo(struct block_device *bdev, struct hd_geometry *geo) {
    geo->heads = 1;
    geo->sectors = DEVICE_SIZE / (geo->heads * KERNEL_SECTOR_SIZE);
    geo->cylinders = DEVICE_SIZE / (geo->heads * geo->sectors * KERNEL_SECTOR_SIZE);
    return 0;
}

static ssize_t my_read(struct block_device *bdev, sector_t sector, void *buffer, size_t size, loff_t *offset) {
    size_t nbytes = size * KERNEL_SECTOR_SIZE;
    memcpy(buffer, my_dev.data + (sector * KERNEL_SECTOR_SIZE), nbytes);
    return nbytes;
}

static ssize_t my_write(struct block_device *bdev, sector_t sector, const void *buffer, size_t size, loff_t *offset) {
    size_t nbytes = size * KERNEL_SECTOR_SIZE;
    memcpy(my_dev.data + (sector * KERNEL_SECTOR_SIZE), buffer, nbytes);
    return nbytes;
}

static struct block_device_operations my_ops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .getgeo = my_getgeo,
    .read = my_read,
    .write = my_write,
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
        printk(KERN_ERR "Failed to register block device\n");
        goto out_unregister;
    }

    my_dev.gd->queue = my_dev.queue;
    my_dev.gd->private_data = &my_dev;
    strcpy(my_dev.gd->disk_name, DEVICE_NAME);
    set_capacity(my_dev.gd, DEVICE_SIZE / KERNEL_SECTOR_SIZE);

    my_dev.gd->fops = &my_ops;
    add_disk(my_dev.gd);

    printk(KERN_INFO "Block device registered\n");
    return 0;

out_unregister:
    unregister_blkdev(my_dev.gd->major, DEVICE_NAME);
out:
    vfree(my_dev.data);
    return -ENOMEM;
}

static void __exit my_block_device_exit(void) {
    printk(KERN_INFO "Exiting my_block_device_exit\n");

    del_gendisk(my_dev.gd);
    printk(KERN_INFO "Gendisk deleted\n");

    put_disk(my_dev.gd);
    printk(KERN_INFO "Disk put\n");

    unregister_blkdev(my_dev.gd->major, DEVICE_NAME);
    printk(KERN_INFO "Block device unregistered\n");

    blk_cleanup_queue(my_dev.queue);
    printk(KERN_INFO "Queue cleaned up\n");

    vfree(my_dev.data);
    printk(KERN_INFO "Data freed\n");

    printk(KERN_INFO "Block device unregistered\n");
}

module_init(my_block_device_init);
module_exit(my_block_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple block device module");
