#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/fs.h>

#define DRIVER_NAME "spi0_bmi160"
#define BMI160_CHIP_ID_REG 0x00
#define EXPECTED_CHIP_ID 0xD1  // BMI160 expected Chip ID

static struct spi_device *bmi160_spi_device = NULL;

static int bmi160_read_chip_id(void)
{
    u8 tx_buf[2] = {BMI160_CHIP_ID_REG | 0x80, 0x00}; // Read command (MSB = 1)
    u8 rx_buf[2] = {0, 0};
    struct spi_transfer t[] = {
        {
            .tx_buf = tx_buf,
            .rx_buf = rx_buf,
            .len = 2,
            .cs_change = 0,
        },
    };

    struct spi_message m;

    pr_info(DRIVER_NAME ": Initializing SPI message");
    spi_message_init(&m);
    spi_message_add_tail(t, &m);

    pr_info(DRIVER_NAME ": Sending SPI transaction - Reading Chip ID");
    if (spi_sync(bmi160_spi_device, &m) < 0) {
        pr_err(DRIVER_NAME ": SPI communication failed");
        return -EIO;
    }

    pr_info(DRIVER_NAME ": SPI transaction completed");
    pr_info(DRIVER_NAME ": Transmitted: 0x%02X 0x%02X", tx_buf[0], tx_buf[1]);
    pr_info(DRIVER_NAME ": Received: 0x%02X 0x%02X", rx_buf[0], rx_buf[1]);

    if (rx_buf[1] != EXPECTED_CHIP_ID) {
        pr_err(DRIVER_NAME ": Unexpected Chip ID: 0x%02X (Expected: 0x%02X)", rx_buf[1], EXPECTED_CHIP_ID);
        return -EINVAL;
    }

    pr_info(DRIVER_NAME ": Read Chip ID successfully: 0x%02X", rx_buf[1]);
    return rx_buf[1]; // Return Chip ID
}

static ssize_t chip_id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    pr_info(DRIVER_NAME ": Reading Chip ID via sysfs");
    int chip_id = bmi160_read_chip_id();
    return sprintf(buf, "0x%02X\n", chip_id);
}

static DEVICE_ATTR(chip_id, 0444, chip_id_show, NULL);

static int bmi160_spi_probe(struct spi_device *spi)
{
    int ret;

    pr_info(DRIVER_NAME ": Probing BMI160 SPI Device");
    bmi160_spi_device = spi;

    pr_info(DRIVER_NAME ": Creating sysfs entry for Chip ID");
    ret = sysfs_create_file(&spi->dev.kobj, &dev_attr_chip_id.attr);
    if (ret) {
        pr_err(DRIVER_NAME ": Failed to create sysfs entry");
        return ret;
    }

    pr_info(DRIVER_NAME ": Probe successful");
    return 0;
}

static int bmi160_spi_remove(struct spi_device *spi)
{
    pr_info(DRIVER_NAME ": Removing SPI device and cleaning up");
    sysfs_remove_file(&spi->dev.kobj, &dev_attr_chip_id.attr);
    return 0;
}

static struct of_device_id bmi160_dt_ids[] = {
    { .compatible = "bosch,bmi160" },
    { }
};
MODULE_DEVICE_TABLE(of, bmi160_dt_ids);

static struct spi_driver bmi160_spi_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = bmi160_dt_ids,
    },
    .probe = bmi160_spi_probe,
    .remove = bmi160_spi_remove,
};

module_spi_driver(bmi160_spi_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("SPI0 Driver for BMI160 on Jetson Nano");
