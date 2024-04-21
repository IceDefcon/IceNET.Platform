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
    printk(KERN_INFO "[FPGA][ B ] alloc_disk\n");
    dev->gd = alloc_disk(MY_BLOCK_MINORS);
    // printk(KERN_INFO "[FPGA][ B ] add_disk\n");
    // add_disk(dev->gd);
}

static int my_block_init(void)
{
    printk(KERN_INFO "[FPGA][ B ] Device Init\n");

    int status;

    status = register_blkdev(MY_BLOCK_MAJOR, MY_BLKDEV_NAME);
    if (status < 0) 
    {
        printk(KERN_INFO "[FPGA][ B ] Unable to register mybdev block device\n");
        return -EBUSY;
    }

    create_block_device(&dev);
}

static void delete_block_device(struct my_block_dev *dev)
{
    if (dev->gd)
    {
        // del_gendisk(dev->gd);
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
