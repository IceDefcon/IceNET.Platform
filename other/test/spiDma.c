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
#include <linux/slab.h>  // For kmalloc and kfree

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
static u8 spi_rx_buf[2] = {0};  // Buffer to store received data (chip ID)
static u8 spi_tx_buf[2] = {0x00, 0x00};  // Buffer to send, 0x00 for read operation
static const int spi_length = 2;  // Buffer size

int spiInit(void)
{
    struct spi_master *spi_master_primary;
    int ret;

    spi_master_primary = spi_busnum_to_master(1);
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
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI0 Allocated\n");
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
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI0 device setup\n");
    }

    return 0;
}

void spiDestroy(void)
{
    spi_dev_put(spi_device_primary);
    printk(KERN_INFO "[DESTROY][SPI] Destroy SPI Devices\n");
}

static int __init spi_module_init(void)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    printk(KERN_INFO "[SPI MODULE] Initializing SPI Kernel Module\n");
    ret = spiInit();
    if (ret < 0) {
        return ret;
    }

    /* Read chip ID from address 0x00 (assuming the device responds with the chip ID at this address) */
    memset(&transfer, 0, sizeof(transfer));

    // Setup for reading from register 0x00
    spi_tx_buf[0] = 0x80;  // The address you want to read from (chip ID register at 0x00)
    spi_tx_buf[1] = 0x00;  // You can fill the next byte with 0 for the read operation

    transfer.tx_buf = (void *)spi_tx_buf;
    transfer.rx_buf = (void *)spi_rx_buf;
    transfer.len = spi_length;

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_device_primary, &msg);
    if (ret < 0)
    {
        printk(KERN_ERR "[CTRL][SPI] SPI transfer failed during initialization: %d\n", ret);
        return ret;
    }
    else
    {
        printk(KERN_INFO "[CTRL][SPI] Primary FPGA Transfer completed during initialization\n");
    }

    /* Read chip ID response from the device */
    printk(KERN_INFO "[CTRL][SPI] Chip ID Read Response: 0x%02x 0x%02x \n", spi_rx_buf[0], spi_rx_buf[1]);

    /* Clear the buffers */
    for (i = 0; i < spi_length; ++i)
    {
        spi_rx_buf[i] = 0x00;
        spi_tx_buf[i] = 0x00;
    }

    return 0;
}

static void __exit spi_module_exit(void)
{
    printk(KERN_INFO "[SPI MODULE] Exiting SPI Kernel Module\n");
    spiDestroy();
}

module_init(spi_module_init);
module_exit(spi_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice.Marek");
MODULE_DESCRIPTION("SPI Kernel Module for Primary FPGA Communication");
