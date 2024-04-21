#include <linux/fs.h>
#include <linux/genhd.h>

#define ICE_BLOCK_MINORS    1
#define ICE_BLOCK_MAJOR     240
#define ICE_BLKDEV_NAME     "iceBlock"

MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice Marek");
MODULE_DESCRIPTION("Block Device Object");

static struct iceBlockDevice 
{
    struct gendisk *gd;
} dev;

static int create_block_device(struct iceBlockDevice *dev)
{
    dev->gd = alloc_disk(ICE_BLOCK_MINORS);
    if (!dev->gd)
    {
        printk(KERN_ERR "alloc_disk failed\n");
        return -ENOMEM;
    }
    
    add_disk(dev->gd);
    return 0;
}

static void delete_block_device(struct iceBlockDevice *dev)
{
    if (dev->gd)
    {
        del_gendisk(dev->gd);
        put_disk(dev->gd);
    }
}

static int iceBlock_init(void)
{
    int status;

    status = register_blkdev(ICE_BLOCK_MAJOR, ICE_BLKDEV_NAME);
    if (status < 0) 
    {
        printk(KERN_ERR "unable to register iceBlock block device\n");
        return -EBUSY;
    }

    status = create_block_device(&dev);
    if (status)
    {
        unregister_blkdev(ICE_BLOCK_MAJOR, ICE_BLKDEV_NAME);
        return status;
    }

    return 0;
}

static void iceBlock_exit(void)
{
    delete_block_device(&dev);
    unregister_blkdev(ICE_BLOCK_MAJOR, ICE_BLKDEV_NAME);
}

module_init(block_device_init);
module_exit(block_device_exit);
