#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spi/spi.h>

static struct spi_device *spi_device;

static int spi_probe(struct spi_device *spi)
{
    printk(KERN_INFO "[FPGA][SPI] Driver probe\n");
    spi_device = spi;
    return 0;
}

static int spi_remove(struct spi_device *spi)
{
    printk(KERN_INFO "[FPGA][SPI] Driver remove\n");
    spi_device = NULL;
    return 0;
}

static struct spi_driver spi_driver_api = 
{
    .driver = 
    {
        .name = "spi-driver",
        .owner = THIS_MODULE,
    },
    .probe = spi_probe,
    .remove = spi_remove,
};

static int __init fpga_driver_init(void)
{
    int ret;
    printk(KERN_INFO "[FPGA][SPI] Driver init\n");
    ret = spi_register_driver(&spi_driver_api);
    
    if (ret < 0) 
    {
        printk(KERN_ERR "Failed to register SPI driver\n");
        return ret;
    }
    return 0;
}

static void __exit fpga_driver_exit(void)
{
    printk(KERN_INFO "[FPGA][SPI] Driver exit\n");
    spi_unregister_driver(&spi_driver_api);
}

module_init(fpga_driver_init);
module_exit(fpga_driver_exit);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("[FPGA][SPI] Driver");
MODULE_LICENSE("GPL");
