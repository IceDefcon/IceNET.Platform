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
#include <linux/delay.h>

#define AD9361_REG_COUNT 0x008
// #define AD9361_REG_COUNT 0x400  // 1024 registers from 0x0000 to 0x03FF

static struct spi_device *spi_device_primary = NULL;
static u8 spi_rx_buf[8] = {0};
static u8 spi_tx_buf[8] = {0};
static const int spi_length = 3;

// -----------------------------------------------------------------------------
// Write to AD9361 register (not used in this version, kept for completeness)
// -----------------------------------------------------------------------------
int ad9361_write_register(u16 reg_addr, u8 value)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;

    memset(&transfer, 0, sizeof(transfer));

    spi_tx_buf[0] = 0x80 | ((reg_addr >> 8) & 0x3F);  // Write operation
    spi_tx_buf[1] = reg_addr & 0xFF;
    spi_tx_buf[2] = value;

    transfer.tx_buf = spi_tx_buf;
    transfer.len = spi_length;

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_device_primary, &msg);
    if (ret < 0) {
        printk(KERN_ERR "[SPI][AD9361] Write to reg 0x%04x failed: %d\n", reg_addr, ret);
        return ret;
    }

    printk(KERN_INFO "[SPI][AD9361] Wrote 0x%02x to reg 0x%04x\n", value, reg_addr);
    return 0;
}

// -----------------------------------------------------------------------------
// Read from AD9361 register
// -----------------------------------------------------------------------------
int ad9361_read_register(u16 reg_addr, u8 *reg_val)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;

    memset(&transfer, 0, sizeof(transfer));

    spi_tx_buf[0] = (reg_addr >> 8) & 0x3F;
    spi_tx_buf[1] = reg_addr & 0xFF;
    spi_tx_buf[2] = 0x00;

    transfer.tx_buf = spi_tx_buf;
    transfer.rx_buf = spi_rx_buf;
    transfer.len = spi_length;

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_device_primary, &msg);
    if (ret < 0)
    {
        printk(KERN_ERR "[SPI][AD9361] Read from reg 0x%04x failed: %d\n", reg_addr, ret);
        return ret;
    }

    *reg_val = spi_rx_buf[2]; // Data returned in byte 2
    printk(KERN_INFO "[AD9361] Reg[0x%04X] = 0x%02X\n", reg_addr, *reg_val);
    return 0;
}

// -----------------------------------------------------------------------------
// SPI Initialization
// -----------------------------------------------------------------------------
int spiInit(void)
{
    struct spi_master *spi_master_primary;
    int ret;

    spi_master_primary = spi_busnum_to_master(0);
    if (!spi_master_primary) {
        printk(KERN_ERR "[INIT][SPI] SPI Master at BUS 0 not found!\n");
        return -ENODEV;
    }

    spi_device_primary = spi_alloc_device(spi_master_primary);
    if (!spi_device_primary) {
        printk(KERN_ERR "[INIT][SPI] SPI0 Failed to Allocate!\n");
        return -ENOMEM;
    }

    spi_device_primary->chip_select = 0;
    spi_device_primary->mode = SPI_MODE_0;
    spi_device_primary->bits_per_word = 8;
    spi_device_primary->max_speed_hz = 10000000;

    ret = spi_setup(spi_device_primary);
    if (ret < 0) {
        printk(KERN_ERR "[INIT][SPI] SPI0 device Failed to setup! ret[%d]\n", ret);
        spi_dev_put(spi_device_primary);
        return ret;
    }

    printk(KERN_INFO "[INIT][SPI] SPI0 device setup complete\n");
    return 0;
}

// -----------------------------------------------------------------------------
// SPI Cleanup
// -----------------------------------------------------------------------------
void spiDestroy(void)
{
    spi_dev_put(spi_device_primary);
    printk(KERN_INFO "[DESTROY][SPI] Destroyed SPI Device\n");
}

// -----------------------------------------------------------------------------
// Kernel Module Init
// -----------------------------------------------------------------------------
static int __init spi_module_init(void)
{
    int ret;
    u16 reg_addr;
    u8 reg_val;

    printk(KERN_INFO "[SPI MODULE] Initializing AD9361 SPI Kernel Module\n");

    ret = spiInit();
    if (ret < 0)
    {
        return ret;
    }

    msleep(10); // Let hardware settle
    // ad9361_write_register(0x0000, 0x01);
    // msleep(100);
    // ad9361_write_register(0x0001, 0x02);
    // msleep(100);
    // ad9361_write_register(0x0002, 0x03);
    // msleep(100);
    // ad9361_write_register(0x0003, 0x04);
    // msleep(100);
    // ad9361_write_register(0x0004, 0x05);
    // msleep(100);
    // ad9361_write_register(0x0005, 0x06);
    // msleep(100);
    // ad9361_write_register(0x0006, 0x07);
    // msleep(100);
    // ad9361_write_register(0x0007, 0x08);
    // msleep(100);

    for (reg_addr = 0x0000; reg_addr < AD9361_REG_COUNT; reg_addr++)
    {
        ret = ad9361_read_register(reg_addr, &reg_val);
        msleep(1); // Slight delay between reads
    }

    printk(KERN_INFO "[AD9361] Register dump complete\n");

    return 0;
}

// -----------------------------------------------------------------------------
// Kernel Module Exit
// -----------------------------------------------------------------------------
static void __exit spi_module_exit(void)
{
    printk(KERN_INFO "[SPI MODULE] Exiting AD9361 SPI Kernel Module\n");
    spiDestroy();
}

module_init(spi_module_init);
module_exit(spi_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice.Marek");
MODULE_DESCRIPTION("SPI Kernel Module for Reading All AD9361 Registers");
