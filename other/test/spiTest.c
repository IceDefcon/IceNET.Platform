#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>

#define BMI160_REG_CHIP_ID  0x00
#define BMI160_CHIP_ID_VAL  0xD1

static struct spi_device *bmi160_spi;

/* Function to read a register from BMI160 */
static int bmi160_read_reg(struct spi_device *spi, u8 reg, u8 *val)
{
    struct spi_transfer xfers[2];
    u8 tx_buf = reg | 0x80;  // MSB=1 for read
    u8 rx_buf = 0;

    memset(xfers, 0, sizeof(xfers));

    xfers[0].tx_buf = &tx_buf;
    xfers[0].rx_buf = NULL;
    xfers[0].len = 1;

    xfers[1].tx_buf = NULL;
    xfers[1].rx_buf = &rx_buf;
    xfers[1].len = 1;

    spi_sync_transfer(spi, xfers, 2);

    *val = rx_buf;
    return 0;
}

/* Probe function - called when the device is found */
static int bmi160_probe(struct spi_device *spi)
{
    u8 chip_id;

    bmi160_spi = spi;

    /* Read the Chip ID */
    if (bmi160_read_reg(spi, BMI160_REG_CHIP_ID, &chip_id)) {
        dev_err(&spi->dev, "Failed to read chip ID\n");
        return -EIO;
    }

    if (chip_id == BMI160_CHIP_ID_VAL) {
        dev_info(&spi->dev, "BMI160 detected! Chip ID: 0x%X\n", chip_id);
    } else {
        dev_err(&spi->dev, "Invalid Chip ID: 0x%X (Expected: 0xD1)\n", chip_id);
        return -ENODEV;
    }

    return 0;
}

/* Remove function - cleanup */
static int bmi160_remove(struct spi_device *spi)
{
    dev_info(&spi->dev, "BMI160 SPI module removed\n");
    return 0;
}

/* SPI Device ID Table */
static const struct of_device_id bmi160_dt_ids[] = {
    { .compatible = "bosch,bmi160" },
    { }
};
MODULE_DEVICE_TABLE(of, bmi160_dt_ids);

/* SPI Driver Structure */
static struct spi_driver bmi160_driver = {
    .driver = {
        .name = "bmi160_spi",
        .of_match_table = bmi160_dt_ids,
    },
    .probe  = bmi160_probe,
    .remove = bmi160_remove,
};

/* Module Initialization */
static int __init bmi160_init(void)
{
    return spi_register_driver(&bmi160_driver);
}

/* Module Exit */
static void __exit bmi160_exit(void)
{
    spi_unregister_driver(&bmi160_driver);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("BMI160 SPI Driver for BeagleBone Black");
MODULE_VERSION("0.1");

module_init(bmi160_init);
module_exit(bmi160_exit);
