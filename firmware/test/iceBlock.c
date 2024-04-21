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

static struct my_block_dev 
{
    struct gendisk *gd;
} dev;

static int create_block_device(struct my_block_dev *dev)
{
    dev->gd = alloc_disk(MY_BLOCK_MINORS);
    add_disk(dev->gd);
}

static int my_block_init(void)
{
    int status;

    status = register_blkdev(MY_BLOCK_MAJOR, MY_BLKDEV_NAME);
    if (status < 0) 
    {
        printk(KERN_ERR "unable to register mybdev block device\n");
        return -EBUSY;
    }

    create_block_device(&dev);
}

static void my_block_exit(void)
{
    unregister_blkdev(MY_BLOCK_MAJOR, MY_BLKDEV_NAME);
}

module_init(my_block_init);
module_exit(my_block_exit);
