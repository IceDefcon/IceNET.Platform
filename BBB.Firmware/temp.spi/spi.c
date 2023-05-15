#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spi/spi.h>

static struct spi_device *spi_device;

static int beaglebone_spi_probe(struct spi_device *spi)
{
    printk(KERN_INFO "BeagleBone SPI driver probe\n");
    spi_device = spi;
    return 0;
}

static int beaglebone_spi_remove(struct spi_device *spi)
{
    printk(KERN_INFO "BeagleBone SPI driver remove\n");
    spi_device = NULL;
    return 0;
}

static struct spi_driver beaglebone_spi_driver = {
    .driver = {
        .name = "beaglebone-spi",
        .owner = THIS_MODULE,
    },
    .probe = beaglebone_spi_probe,
    .remove = beaglebone_spi_remove,
};

static int __init beaglebone_spi_init(void)
{
    int ret;
    printk(KERN_INFO "BeagleBone SPI driver init\n");
    ret = spi_register_driver(&beaglebone_spi_driver);
    if (ret < 0) {
        printk(KERN_ERR "Failed to register SPI driver\n");
        return ret;
    }
    return 0;
}

static void __exit beaglebone_spi_exit(void)
{
    printk(KERN_INFO "BeagleBone SPI driver exit\n");
    spi_unregister_driver(&beaglebone_spi_driver);
}

module_init(beaglebone_spi_init);
module_exit(beaglebone_spi_exit);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("BeagleBone SPI driver");
MODULE_LICENSE("GPL");
