#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/mutex.h>    // Include for mutex opearations

#define DEVICE_NAME "iceBLOCK"
#define DEVICE_SIZE (1024 * 1024) // 1MB
#define KERNEL_SECTOR_SIZE 512
#define DEVICE_MINORS 1

static int numberOpens = 0;
static DEFINE_MUTEX(com_mutex);

static struct iceBlockDevice 
{
    unsigned char *data;
    struct request_queue *queue;
    struct gendisk *gd;
} iceBlock;

static int dev_open(struct block_device *bdev, fmode_t mode) 
{
    if(!mutex_trylock(&com_mutex))
    {
        printk(KERN_ALERT "[FPGA][ B ] Device in use by another process");
        return -EBUSY;
    }

    numberOpens++;
    printk(KERN_INFO "[FPGA][ B ] Device has been opened %d time(s)\n", numberOpens);
    return 0;
}

static void dev_release(struct gendisk *disk, fmode_t mode) 
{
    mutex_unlock(&com_mutex);
    printk(KERN_INFO "[FPGA][ B ] Device successfully closed\n");
    return;
}

static struct block_device_operations my_ops = 
{
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
};

static int __init block_device_init(void) {

    printk(KERN_INFO "[FPGA][ B ] -----===[ INIT ]===-----\n");
    printk(KERN_INFO "[FPGA][ B ] Allocate 1MB of kernel memory to store block device\n");
    iceBlock.data = vmalloc(DEVICE_SIZE);
    if (!iceBlock.data)
    {
        printk(KERN_INFO "[FPGA][ B ] Failed to allocate 1MB kernel memory!\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "[FPGA][ B ] Allocate request queue for a block device\n");
    iceBlock.queue = blk_alloc_queue(GFP_KERNEL);
    if (!iceBlock.queue)
    {
        printk(KERN_INFO "[FPGA][ B ] Failed to request for queue for a block device!\n");
        vfree(iceBlock.data);
        return -ENOMEM;
    }

    /**
     * 
     * Sets the logical block size for the block device's request 
     * 
     * Such that now we have 1MB / 512B = 2048 blocks
     * Each block can be used indepenently for RD/WR operation
     * 
     */
    printk(KERN_INFO "[FPGA][ B ] Sets the logical block size 512B for a single RD/WR request \n");
    blk_queue_logical_block_size(iceBlock.queue, KERNEL_SECTOR_SIZE);

    /**
     * 
     * Allocate a new gendisk structure for a block device
     * 
     * Contains information about the device, 
     * such as its major and minor numbers, 
     * request queue, and operations
     * 
     * DEVICE_MINORS == 1
     * Single block device without partitions
     * 
     */
    printk(KERN_INFO "[FPGA][ B ] Allocate gendisk structure for a Single block device without partitions \n");
    iceBlock.gd = alloc_disk(DEVICE_MINORS);
    if (!iceBlock.gd)
    {
        printk(KERN_INFO "[FPGA][ B ] Failed to allocating gen disk for a block device!\n");
        vfree(iceBlock.data);
        return -ENOMEM;
    }

    printk(KERN_INFO "[FPGA][ B ] Register block device...\n");
    iceBlock.gd->major = register_blkdev(0, DEVICE_NAME);

    if (iceBlock.gd->major < 0) 
    {
        printk(KERN_INFO "[FPGA][ B ] Failed to register block device with error: %d\n", iceBlock.gd->major);
        unregister_blkdev(iceBlock.gd->major, DEVICE_NAME);
    }

    printk(KERN_INFO "[FPGA][ B ] Register block device with major number: %d\n", iceBlock.gd->major);

    iceBlock.gd->queue = iceBlock.queue;
    iceBlock.gd->private_data = &iceBlock;
    strcpy(iceBlock.gd->disk_name, DEVICE_NAME);
    set_capacity(iceBlock.gd, DEVICE_SIZE / KERNEL_SECTOR_SIZE);

    iceBlock.gd->fops = &my_ops;
#if 0
    add_disk(iceBlock.gd);
#endif
    printk(KERN_INFO "[FPGA][ B ] Block device registered SUCCESS\n");

    mutex_init(&com_mutex);

    return 0;
}

static void __exit block_device_exit(void)
{
    printk(KERN_INFO "[FPGA][ B ] -----===[ EXIT ]===-----\n");
#if 0
    if (iceBlock.gd) 
    {
        printk(KERN_INFO "[FPGA][ B ] Deleting gendisk with major number %d\n", iceBlock.gd->major);
        del_gendisk(iceBlock.gd);
    } 
    else 
    {
        printk(KERN_WARNING "Gendisk does not exist\n");
    }
#endif
    if (iceBlock.queue) 
    {
        printk(KERN_INFO "[FPGA][ B ] Cleaning up block device queue");
        blk_cleanup_queue(iceBlock.queue);
    } 
    else 
    {
        printk(KERN_WARNING "Queue does not exist\n");
    }

    if (iceBlock.gd) 
    {
        printk(KERN_INFO "[FPGA][ B ] Unregistering block device with major number %d\n", iceBlock.gd->major);
        unregister_blkdev(iceBlock.gd->major, DEVICE_NAME);
    } 
    else 
    {
        printk(KERN_WARNING "[FPGA][ B ] Gendisk does not exist for unregistering\n");
    }
#if 0
    if (iceBlock.gd) 
    {
        printk(KERN_INFO "[FPGA][ B ] Decrements the reference count of a struct gendisk object");
        put_disk(iceBlock.gd);
    } 
    else 
    {
        printk(KERN_WARNING "[FPGA][ B ] Gendisk does not exist for putting\n");
    }
#endif
    if (iceBlock.data) 
    {
        printk(KERN_INFO "[FPGA][ B ] DeAllocate 1MB of kernel memory");
        vfree(iceBlock.data);
    } 
    else 
    {
        printk(KERN_WARNING "[FPGA][ B ] Data does not exist\n");
    }

    printk(KERN_INFO "[FPGA][ B ] Block device exit completed");
    mutex_destroy(&com_mutex);
}

module_init(block_device_init);
module_exit(block_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple block device module");
