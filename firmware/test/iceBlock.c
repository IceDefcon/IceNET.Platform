#include <linux/fs.h>
#include <linux/init.h>  // Include for module_init and module_exit
#include <linux/genhd.h>
#include <linux/module.h>  // Include the module header file

MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice Marek");
MODULE_DESCRIPTION("Block Device Object");

#define MY_BLOCK_MAJOR           240
#define MY_BLKDEV_NAME          "mybdev"
#define MY_BLOCK_MINORS       1

#define NR_SECTORS                   1024
#define KERNEL_SECTOR_SIZE           512

static struct my_block_dev {
    spinlock_t lock;                /* For mutual exclusion */
    struct request_queue *queue;    /* The device request queue */
    struct gendisk *gd;             /* The gendisk structure */
} dev;

static int create_block_device(struct my_block_dev *dev)
{
    /* Initialize the gendisk structure */
    dev->gd = alloc_disk(MY_BLOCK_MINORS);
    if (!dev->gd) {
        printk (KERN_NOTICE "alloc_disk failure\n");
        return -ENOMEM;
    }

    dev->gd->major = MY_BLOCK_MAJOR;
    dev->gd->first_minor = 0;
    dev->gd->fops = &my_block_dev;
    dev->gd->queue = dev->queue;
    dev->gd->private_data = dev;
    snprintf (dev->gd->disk_name, 32, "myblock");
    set_capacity(dev->gd, NR_SECTORS);

    add_disk(dev->gd);

    return 0;
}

static int my_block_init(void)
{
    printk(KERN_INFO "[FPGA][ B ] Device Init\n");

    int status;

    status = register_blkdev(MY_BLOCK_MAJOR, MY_BLKDEV_NAME);
    if (status < 0) 
    {
        printk(KERN_INFO "[FPGA][ B ] Unable to register mybdev block device\n");
        /* in case if major number is taken */
        unregister_blkdev(MY_BLOCK_MAJOR, MY_BLKDEV_NAME);
        return -EBUSY;
    }

    status = create_block_device(&dev);
    if (status < 0)
    {
        return status;
    }
}

static void delete_block_device(struct my_block_dev *dev)
{
    if (dev->gd)
    {
        del_gendisk(dev->gd);
    }
}

static void my_block_exit(void)
{
    delete_block_device(&dev);
    printk(KERN_INFO "[FPGA][ B ] Unregister mybdev\n");
    unregister_blkdev(MY_BLOCK_MAJOR, MY_BLKDEV_NAME);
    printk(KERN_INFO "[FPGA][ B ] Device Exit Successfuly\n");
}

module_init(my_block_init);
module_exit(my_block_exit);
