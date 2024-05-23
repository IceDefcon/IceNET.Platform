#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>

#define SPI_BUS 1          // SPI bus number (for SPI1)
#define SPI_BUS_CS1 1      // Chip select (CS) number
#define SPI_BUS_SPEED 50000 // SPI speed in Hz
#define REGISTER_ADDRESS 0x00

static struct spi_device *spi_device;

static int __init spi_example_init(void)
{
    struct spi_master *master;
    struct spi_board_info spi_device_info = {
        .modalias = "spi_example_device",
        .max_speed_hz = SPI_BUS_SPEED,
        .bus_num = SPI_BUS,
        .chip_select = SPI_BUS_CS1,
        .mode = SPI_MODE_0,
    };

    master = spi_busnum_to_master(SPI_BUS);
    if (!master) {
        pr_err("MASTER not found.\n");
        return -ENODEV;
    }

    spi_device = spi_new_device(master, &spi_device_info);
    if (!spi_device) {
        pr_err("FAILED to create slave.\n");
        return -ENODEV;
    }

    spi_device->bits_per_word = 8;
    if (spi_setup(spi_device)) {
        pr_err("FAILED to setup slave.\n");
        spi_unregister_device(spi_device);
        return -ENODEV;
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

    spi_message_init(&message);
    spi_message_add_tail(&transfer, &message);

    // Send the message
    spi_sync(spi_device, &message);

    pr_info("Register 0x00 value: 0x%02X\n", rx_buffer[1]);

    return 0;
}

static void __exit spi_example_exit(void)
{
    spi_unregister_device(spi_device);
    pr_info("SPI example module exited.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("SPI example kernel module for reading register 0x00");

module_init(spi_example_init);
module_exit(spi_example_exit);
