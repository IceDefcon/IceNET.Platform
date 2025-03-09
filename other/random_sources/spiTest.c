/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/slab.h>

#define SPI_BUS 0
#define SPI_CHIP_SELECT 0
#define SPI_MAX_SPEED 1000000
#define BMI160_CHIP_ID_REG 0x00
#define CHIP_ID_LEN 1

static struct spi_device *spi_dev;

static int spi_read_chip_id(void)
{
    unsigned char tx_buf[2] = {BMI160_CHIP_ID_REG, 0x00};
    unsigned char rx_buf[2] = {0};
    struct spi_transfer t = {
        .tx_buf = tx_buf,
        .rx_buf = rx_buf,
        .len = 2,
        .speed_hz = SPI_MAX_SPEED,
        .cs_change = 1,
    };
    struct spi_message m;

    spi_message_init(&m);
    spi_message_add_tail(&t, &m);

    if (spi_sync(spi_dev, &m) < 0)
    {
        printk(KERN_ERR "[SPI] Failed to read BMI160 Chip ID\n");
        return -EIO;
    }

    printk(KERN_INFO "[SPI] BMI160 Chip ID: 0x%02X\n", rx_buf[1]);
    return 0;
}

static int __init spi_init(void)
{
    struct spi_master *master;

    master = spi_busnum_to_master(SPI_BUS);
    if (!master)
    {
        printk(KERN_ERR "[INIT][SPI] SPI Master Bus not found!\n");
        return -ENODEV;
    }

    spi_dev = spi_alloc_device(master);
    if (!spi_dev)
    {
        printk(KERN_ERR "[INIT][SPI] SPI Device Allocation Failed!\n");
        return -ENOMEM;
    }

    spi_dev->chip_select = SPI_CHIP_SELECT;
    spi_dev->mode = SPI_MODE_0;
    spi_dev->bits_per_word = 8;
    spi_dev->max_speed_hz = SPI_MAX_SPEED;

    if (spi_setup(spi_dev) < 0)
    {
        printk(KERN_ERR "[INIT][SPI] SPI Device Setup Failed!\n");
        spi_dev_put(spi_dev);
        return -EIO;
    }

    printk(KERN_INFO "[INIT][SPI] SPI Device Initialized Successfully\n");
    return spi_read_chip_id();
}

static void __exit spi_exit(void)
{
    if (spi_dev)
    {
        spi_dev_put(spi_dev);
        printk(KERN_INFO "[EXIT][SPI] SPI Device Released\n");
    }
}

module_init(spi_init);
module_exit(spi_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice.Marek");
MODULE_DESCRIPTION("SPI Kernel Module to Read BMI160 Chip ID");
