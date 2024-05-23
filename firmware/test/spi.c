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
        .len = 1,
    };
    struct spi_message message;

    // Fill the tx_buffer with the bytes to send
    tx_buffer[0] = 0x7F;


    // Send the message
    spi_message_init(&message);
    spi_message_add_tail(&transfer, &message);

    ret = spi_sync(spi_dev_primary, &message);
    if (ret) {
        printk(KERN_ERR "[INIT][SPI] SPI read failed.\n");
        spi_unregister_device(spi_dev_primary);
        put_device(&spi_master_primary->dev);  // Clean up master reference
        return ret;
    }

    printk(KERN_INFO "SPI transfer completed, received bytes: 0x%02X 0x%02X\n", rx_buffer[0]);

    // Fill the tx_buffer with the bytes to send
    tx_buffer[0] = 0x00;


    // Send the message
    spi_message_init(&message);
    spi_message_add_tail(&transfer, &message);

    ret = spi_sync(spi_dev_primary, &message);
    if (ret) {
        printk(KERN_ERR "[INIT][SPI] SPI read failed.\n");
        spi_unregister_device(spi_dev_primary);
        put_device(&spi_master_primary->dev);  // Clean up master reference
        return ret;
    }

    printk(KERN_INFO "SPI transfer completed, received bytes: 0x%02X 0x%02X\n", rx_buffer[0]);

    return 0;
}

static void __exit spi_example_exit(void)
{
    spi_dev_put(spi_dev_primary);
    pr_info("SPI example module exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("SPI example kernel module for reading register 0x00");

module_init(spi_example_init);
module_exit(spi_example_exit);


