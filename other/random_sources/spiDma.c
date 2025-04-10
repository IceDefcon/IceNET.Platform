/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/delay.h>\

////////////////////////
//                    //
//                    //
//                    //
//   [SPI] Control    //
//                    //
//                    //
//                    //
////////////////////////

static struct spi_device *spi_device_primary = NULL;
static u8 spi_rx_buf[8] = {0};
static u8 spi_tx_buf[8] = {0};
static const int spi_length = 2;

int bmi160_write_register(u8 reg, u8 data)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;

    memset(&transfer, 0, sizeof(transfer));

    spi_tx_buf[0] = reg;
    spi_tx_buf[1] = data;

    transfer.tx_buf = (void *)spi_tx_buf;
    transfer.len = spi_length;

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_device_primary, &msg);
    if (ret < 0)
    {
        printk(KERN_ERR "[CTRL][SPI] Write to register 0x%02x failed: %d\n", reg, ret);
        return ret;
    }

    printk(KERN_INFO "[CTRL][SPI] Wrote 0x%02x to register 0x%02x\n", data, reg);
    return 0;
}

int bmi160_read_register(u8 reg, u8 *data)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;

    memset(&transfer, 0, sizeof(transfer));

    spi_tx_buf[0] = reg | 0x80;
    transfer.tx_buf = (void *)spi_tx_buf;
    transfer.rx_buf = (void *)spi_rx_buf;
    transfer.len = spi_length;

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_device_primary, &msg);
    if (ret < 0)
    {
        printk(KERN_ERR "[CTRL][SPI] Read from register 0x%02x failed: %d\n", reg, ret);
        return ret;
    }

    *data = spi_rx_buf[1];
    printk(KERN_INFO "[CTRL][SPI] Read 0x%02x from register 0x%02x\n", *data, reg);
    return 0;
}

int spiInit(void)
{
    struct spi_master *spi_master_primary;
    int ret;

    spi_master_primary = spi_busnum_to_master(0);
    if (!spi_master_primary)
    {
        printk(KERN_ERR "[INIT][SPI] SPI Master at BUS 0 not found!\n");
        return -ENODEV;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI Master at BUS 0 Registered\n");
    }

    spi_device_primary = spi_alloc_device(spi_master_primary);
    if (!spi_device_primary)
    {
        printk(KERN_ERR "[INIT][SPI] SPI0 Failed to Allocate!\n");
        return -ENOMEM;
    }

    spi_device_primary->chip_select = 0;
    spi_device_primary->mode = SPI_MODE_0;
    spi_device_primary->bits_per_word = 8;
    spi_device_primary->max_speed_hz = 1000000;

    ret = spi_setup(spi_device_primary);
    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][SPI] SPI0 device Failed to setup! ret[%d]\n", ret);
        spi_dev_put(spi_device_primary);
        return ret;
    }

    printk(KERN_ERR "[INIT][SPI] SPI0 device setup\n");
    return 0;
}

void spiDestroy(void)
{
    spi_dev_put(spi_device_primary);
    printk(KERN_INFO "[DESTROY][SPI] Destroy SPI Devices\n");
}

static int __init spi_module_init(void)
{
    int ret;
    u8 chip_id;
    u8 accel_x_lo, accel_x_hi;
    // u8 accel_y_lo, accel_y_hi;
    // u8 accel_z_lo, accel_z_hi;

    printk(KERN_INFO "[SPI MODULE] Initializing SPI Kernel Module\n");
    ret = spiInit();
    if (ret < 0)
    {
        return ret;
    }

    // Soft reset the sensor
    bmi160_write_register(0x7E, 0xB6); // Soft reset
    msleep(100); // Allow sensor to reset completely

    // Set accelerometer to normal mode
    bmi160_write_register(0x7E, 0x11);
    msleep(50); // Short delay

    // Set gyroscope to normal mode
    bmi160_write_register(0x7E, 0x15);
    msleep(50); // Short delay

    // Set output data rate (ODR), range, etc.
    bmi160_write_register(0x40, 0x2C);
    msleep(10); // Short delay before reading data


    ret = bmi160_read_register(0x00, &chip_id);
    if (ret < 0)
    {
        return ret;
    }

    ret = bmi160_read_register(0x12, &accel_x_lo);
    if (ret < 0)
    {
        return ret;
    }

    ret = bmi160_read_register(0x13, &accel_x_hi);
    if (ret < 0)
    {
        return ret;
    }

    // ret = bmi160_read_register(0x14, &accel_y_lo);
    // if (ret < 0)
    // {
    //     return ret;
    // }

    // ret = bmi160_read_register(0x15, &accel_y_hi);
    // if (ret < 0)
    // {
    //     return ret;
    // }

    // ret = bmi160_read_register(0x16, &accel_z_lo);
    // if (ret < 0)
    // {
    //     return ret;
    // }

    // ret = bmi160_read_register(0x17, &accel_z_hi);
    // if (ret < 0)
    // {
    //     return ret;
    //

    printk(KERN_INFO "[CTRL][SPI] Chip ID = 0x%02x\n", chip_id);
    printk(KERN_INFO "[CTRL][SPI] Acceleration Data: X_lo=0x%02x, X_hi=0x%02x\n", accel_x_lo, accel_x_hi);
    // printk(KERN_INFO "[CTRL][SPI] Acceleration Data: Y_lo=0x%02x, Y_hi=0x%02x\n", accel_y_lo, accel_y_hi);
    // printk(KERN_INFO "[CTRL][SPI] Acceleration Data: Z_lo=0x%02x, Z_hi=0x%02x\n", accel_z_lo, accel_z_hi);

    return 0;
}

// Module exit
static void __exit spi_module_exit(void)
{
    printk(KERN_INFO "[SPI MODULE] Exiting SPI Kernel Module\n");
    spiDestroy();
}

module_init(spi_module_init);
module_exit(spi_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice.Marek");
MODULE_DESCRIPTION("SPI Kernel Module for BMI160 Sensor Communication");
