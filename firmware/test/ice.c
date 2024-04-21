#include <linux/fs.h>
#include <linux/genhd.h>

#define ICE_BLOCK_MINORS    1
#define ICE_BLOCK_MAJOR     240
#define ICE_BLKDEV_NAME     "iceBlock"

static struct iceBlockDevice 
{
    struct gendisk *gd;
} dev;

static int create_block_device(struct iceBlockDevice *dev)
{
    dev->gd = alloc_disk(ICE_BLOCK_MINORS);
    add_disk(dev->gd);
}

static void delete_block_device(struct iceBlockDevice *dev)
{
    if (dev->gd)
    {
        del_gendisk(dev->gd);
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

    create_block_device(&dev);
}

static void iceBlock_exit(void)
{
    delete_block_device(&dev);
    unregister_blkdev(ICE_BLOCK_MAJOR, ICE_BLKDEV_NAME);
}
