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
    struct spi_board_info spi_device_info = {
        .modalias = "spidev",
        .max_speed_hz = 1000000,
        .bus_num = 0,
        .chip_select = 0,
        .mode = SPI_MODE_0,
    };
    int ret;

    spi_master_primary = spi_busnum_to_master(spi_device_info.bus_num);
    if (!spi_master_primary) {
        printk(KERN_ERR "[INIT][SPI] SPI Master at BUS %d not found!\n", spi_device_info.bus_num);
        return -ENODEV;
    } else {
        printk(KERN_INFO "[INIT][SPI] SPI Master at BUS %d Registered\n", spi_device_info.bus_num);
    }

    spi_dev_primary = spi_new_device(spi_master_primary, &spi_device_info);
    if (!spi_dev_primary) {
        printk(KERN_ERR "[INIT][SPI] SPI0 Failed to Allocate!\n");
        put_device(&spi_master_primary->dev);
        return -ENOMEM;
    } else {
        printk(KERN_INFO "[INIT][SPI] SPI0 Allocated\n");
    }

    spi_dev_primary->bits_per_word = 8;

    ret = spi_setup(spi_dev_primary);
    if (ret < 0) {
        printk(KERN_ERR "[INIT][SPI] SPI0 device Failed to setup! ret[%d]\n", ret);
        spi_unregister_device(spi_dev_primary);
        put_device(&spi_master_primary->dev);  // Clean up master reference
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

    printk(KERN_INFO "Register 0x00 value: 0x%02X\n", rx_buffer[1]);

    return 0;
}

static void __exit spi_example_exit(void)
{
    if (spi_dev_primary) {
        struct spi_master *master = spi_dev_primary->master;
        spi_unregister_device(spi_dev_primary);
        put_device(&master->dev);  // Clean up master reference
        spi_dev_primary = NULL;
    }
    pr_info("SPI example module exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("SPI example kernel module for reading register 0x00");

module_init(spi_example_init);
module_exit(spi_example_exit);
