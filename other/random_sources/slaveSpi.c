#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#define SPI_BUS     1   // SPI1 bus on BeagleBone Black
#define SPI_SLAVE_CS 0  // Chip select for the slave

static struct spi_device *spi_slave_device;

static irqreturn_t spi_slave_irq_handler(int irq, void *dev_id)
{
    int i;
    struct spi_message message;
    struct spi_transfer transfer;
    uint8_t rx_buffer[16] = {0}; // Buffer to store received data

    memset(&transfer, 0, sizeof(transfer));
    transfer.rx_buf = rx_buffer;
    transfer.len = sizeof(rx_buffer);
    spi_message_init(&message);
    spi_message_add_tail(&transfer, &message);

    if (spi_sync(spi_slave_device, &message) < 0) {
        pr_err("SPI Slave: Failed to receive data\n");
        return IRQ_HANDLED;
    }

    pr_info("SPI Slave: Received data: ");
    for (i = 0; i < sizeof(rx_buffer); i++)
        pr_cont("%02X ", rx_buffer[i]);
    pr_cont("\n");

    return IRQ_HANDLED;
}

static int spi_slave_probe(struct spi_device *spi)
{
    int irq;
    pr_info("SPI Slave: Probing SPI slave device\n");

    spi_slave_device = spi;
    spi->mode = SPI_MODE_0; // Set SPI mode (adjust if needed)
    spi->bits_per_word = 8;
    spi->max_speed_hz = 1000000; // Adjust based on FPGA speed
    spi_setup(spi);

    irq = gpio_to_irq(spi->irq);
    if (request_irq(irq, spi_slave_irq_handler, IRQF_TRIGGER_FALLING, "spi_slave_irq", NULL)) {
        pr_err("SPI Slave: Failed to request IRQ\n");
        return -1;
    }

    pr_info("SPI Slave: Registered successfully\n");
    return 0;
}

static int spi_slave_remove(struct spi_device *spi)
{
    free_irq(spi->irq, NULL);
    pr_info("SPI Slave: Removed\n");
    return 0;
}

static struct spi_driver spi_slave_driver = {
    .driver = {
        .name = "spi_slave_device",
        .owner = THIS_MODULE,
    },
    .probe = spi_slave_probe,
    .remove = spi_slave_remove,
};

static int __init spi_slave_init(void)
{
    pr_info("SPI Slave: Initializing module\n");
    return spi_register_driver(&spi_slave_driver);
}

static void __exit spi_slave_exit(void)
{
    pr_info("SPI Slave: Exiting module\n");
    spi_unregister_driver(&spi_slave_driver);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("SPI1 Slave Driver for BeagleBone Black");
MODULE_VERSION("1.0");

module_init(spi_slave_init);
module_exit(spi_slave_exit);
