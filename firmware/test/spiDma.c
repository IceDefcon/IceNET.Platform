#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#define DRIVER_NAME "spi_dma_example"

static struct spi_device *spi_device;

static int spi_dma_transfer(struct spi_device *spi, u8 *tx_buf, u8 *rx_buf, size_t len)
{
    struct spi_transfer t = {
        .tx_buf = tx_buf,
        .rx_buf = rx_buf,
        .len = len,
        .speed_hz = spi->max_speed_hz,
        .cs_change = 1,
        .dma_tx = DMA_MAPPING_ERROR,
        .dma_rx = DMA_MAPPING_ERROR,
    };
    struct spi_message m;

    spi_message_init(&m);
    spi_message_add_tail(&t, &m);

    // Initiate the SPI DMA transfer
    int ret = spi_sync(spi, &m);
    if (ret)
        pr_err("SPI DMA transfer failed: %d\n", ret);

    return ret;
}

static int __init spi_dma_example_init(void)
{
    struct spi_master *master;
    int bus_num = 1;  // SPI0 is typically mapped to bus number 1
    int ret;

    // Get the SPI master corresponding to the bus number
    master = spi_busnum_to_master(bus_num);
    if (!master) {
        pr_err("SPI master not found.\n");
        return -ENODEV;
    }

    // Allocate and set up the SPI device structure
    spi_device = spi_alloc_device(master);
    if (!spi_device) {
        put_device(&master->dev);
        pr_err("Failed to allocate SPI device.\n");
        return -ENOMEM;
    }

    spi_device->chip_select = 0; // Use CS0
    spi_device->max_speed_hz = 24000000; // Max SPI speed
    spi_device->mode = SPI_MODE_0;
    spi_device->bits_per_word = 8;

    // Register the SPI device with the SPI core
    ret = spi_add_device(spi_device);
    if (ret) {
        spi_dev_put(spi_device);
        pr_err("Failed to add SPI device.\n");
        return ret;
    }

    pr_info("SPI DMA Example: Device registered successfully.\n");

    // Example data to send and receive
    u8 tx_buf[16] = {0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78,
                     0x9A, 0xBC, 0xDE, 0xF0, 0x11, 0x22, 0x33, 0x44};
    u8 rx_buf[16] = {0};

    // Perform a DMA SPI transfer
    spi_dma_transfer(spi_device, tx_buf, rx_buf, sizeof(tx_buf));

    return 0;
}

static void __exit spi_dma_example_exit(void)
{
    if (spi_device) {
        spi_unregister_device(spi_device);
    }
    pr_info("SPI DMA Example: Device unregistered.\n");
}

module_init(spi_dma_example_init);
module_exit(spi_dma_example_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("BeagleBone Black SPI0 DMA Example Kernel Module");
