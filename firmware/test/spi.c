#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define SPI_BUS 0
#define SPI_BUS_CS1 0  // Assuming chip select 0
#define SPI_BUS_SPEED 50000

static struct spi_device *spi_device;

static int spi_read_register(uint8_t reg, uint8_t *val) {
    uint8_t tx_buf[2] = { reg | 0x80, 0x00 }; // 0x80 to set read mode
    uint8_t rx_buf[2] = { 0 };
    struct spi_transfer transfer = {
        .tx_buf = tx_buf,
        .rx_buf = rx_buf,
        .len = 2,
    };
    struct spi_message message;

    spi_message_init(&message);
    spi_message_add_tail(&transfer, &message);

    if (spi_sync(spi_device, &message)) {
        printk(KERN_ALERT "Failed to transfer SPI message for register 0x%02X.\n", reg);
        return -EIO;
    }

    *val = rx_buf[1];
    return 0;
}

static int __init spi_module_init(void) {
    struct spi_master *master;
    struct spi_board_info spi_device_info = {
        .modalias = "spidev",
        .max_speed_hz = SPI_BUS_SPEED,
        .bus_num = SPI_BUS,
        .chip_select = SPI_BUS_CS1,
        .mode = SPI_MODE_0,
    };
    uint8_t val;

    printk(KERN_INFO "SPI Module Init\n");

    master = spi_busnum_to_master(spi_device_info.bus_num);
    if (!master) {
        printk(KERN_ALERT "SPI Master not found.\n");
        return -ENODEV;
    }

    spi_device = spi_new_device(master, &spi_device_info);
    if (!spi_device) {
        printk(KERN_ALERT "Failed to create SPI device.\n");
        return -ENODEV;
    }

    if (spi_setup(spi_device)) {
        printk(KERN_ALERT "Failed to setup SPI device.\n");
        spi_unregister_device(spi_device);
        return -ENODEV;
    }

    if (spi_read_register(0x7F, &val) == 0) {
        printk(KERN_INFO "Read value from register 0x7F: 0x%02X\n", val);
    }

    if (spi_read_register(0x0F, &val) == 0) {
        printk(KERN_INFO "Read value from register 0x0F: 0x%02X\n", val);
    }

    return 0;
}

static void __exit spi_module_exit(void) {
    printk(KERN_INFO "SPI Module Exit\n");
    spi_unregister_device(spi_device);
}

module_init(spi_module_init);
module_exit(spi_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("SPI Read Kernel Module");
MODULE_VERSION("0.1");
