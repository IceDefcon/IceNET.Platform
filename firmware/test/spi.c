#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>  // For kmalloc and kfree

#define REGISTER_ADDRESS 0x00

static struct spi_device *spi_dev_primary;


static int __init spi_example_init(void)
{
    struct spi_master *spi_master_primary;
    int ret;

    spi_master_primary = spi_busnum_to_master(0);
    if (!spi_master_primary) {
        printk(KERN_ERR "[INIT][SPI] SPI Master at BUS 0 not found!\n");
        return -ENODEV;
    } else {
        printk(KERN_INFO "[INIT][SPI] SPI Master at BUS 0 Registered\n");
    }

    spi_dev_primary = spi_alloc_device(spi_master_primary);
    if (!spi_dev_primary) {
        printk(KERN_ERR "[INIT][SPI] SPI0 Failed to Allocate!\n");
        put_device(&spi_master_primary->dev);
        return -ENOMEM;
    } else {
        printk(KERN_INFO "[INIT][SPI] SPI0 Allocated\n");
    }

    spi_dev_primary->chip_select = 0;
    spi_dev_primary->mode = SPI_MODE_0;
    spi_dev_primary->bits_per_word = 8;
    spi_dev_primary->max_speed_hz = 1000000;
    spi_dev_primary->controller_data = NULL;

    ret = spi_setup(spi_dev_primary);
    if (ret < 0) {
        printk(KERN_ERR "[INIT][SPI] SPI0 device Failed to setup! ret[%d]\n", ret);
        spi_dev_put(spi_dev_primary);
        return ret;
    } else {
        printk(KERN_INFO "[INIT][SPI] SPI0 device setup\n");
    }

    // Allocate memory for the buffer
    uint8_t tx_buffer[2];
    uint8_t rx_buffer[2];
    struct spi_transfer transfer = {
        .tx_buf = tx_buffer,
        .rx_buf = rx_buffer,
        .len = 2,
    };
    struct spi_message message;

    tx_buffer[0] = REGISTER_ADDRESS;
    tx_buffer[1] = 0x00; // Dummy byte

    return 0;
}

static void __exit spi_example_exit(void)
{
    spi_unregister_device(spi_dev_primary);
    pr_info("SPI example module exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("SPI example kernel module for reading register 0x00");

module_init(spi_example_init);
module_exit(spi_example_exit);
