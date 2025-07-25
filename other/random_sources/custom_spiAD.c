/*
 * AD9361 SPI Register Access Module
 * Author: Ice.Marek
 * Updated by ChatGPT (OpenAI) for AD9361 compatibility
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

#define AD9361_REG_COUNT 0x008 // Adjust as needed (0x400 = full range)

static struct spi_device *spi_device_primary = NULL;
static u8 spi_rx_buf[8] = {0};
static u8 spi_tx_buf[8] = {0};
static const int spi_length = 3;

// -----------------------------------------------------------------------------
// Write to AD9361 Register
// -----------------------------------------------------------------------------
int ad9361_write_register(u16 reg_addr, u8 value)
{
    struct spi_message msg;
    struct spi_transfer transfer = {0};
    int ret;

    spi_tx_buf[0] = 0x80 | ((reg_addr >> 8) & 0x3F);
    spi_tx_buf[1] = reg_addr & 0xFF;
    spi_tx_buf[2] = value;

    transfer.tx_buf = spi_tx_buf;
    transfer.len = spi_length;

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_device_primary, &msg);
    if (ret < 0) {
        pr_err("[AD9361][SPI] Write 0x%04X failed: %d\n", reg_addr, ret);
        return ret;
    }

    pr_info("[AD9361] Wrote 0x%02X to reg 0x%04X\n", value, reg_addr);
    return 0;
}

// -----------------------------------------------------------------------------
// Read from AD9361 Register
// -----------------------------------------------------------------------------
int ad9361_read_register(u16 reg_addr, u8 *reg_val)
{
    struct spi_message msg;
    struct spi_transfer transfer = {0};
    int ret;

    memset(spi_rx_buf, 0, sizeof(spi_rx_buf));

    spi_tx_buf[0] = 0x00 | ((reg_addr >> 8) & 0x3F);
    spi_tx_buf[1] = reg_addr & 0xFF;
    spi_tx_buf[2] = 0x00;

    transfer.tx_buf = spi_tx_buf;
    transfer.rx_buf = spi_rx_buf;
    transfer.len = spi_length;

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_device_primary, &msg);
    if (ret < 0) {
        pr_err("[AD9361][SPI] Read 0x%04X failed: %d\n", reg_addr, ret);
        return ret;
    }

    *reg_val = spi_rx_buf[2]; // Data is in byte 2
    pr_info("[AD9361] Reg[0x%04X] = 0x%02X\n", reg_addr, *reg_val);
    return 0;
}

// -----------------------------------------------------------------------------
// SPI Initialization
// -----------------------------------------------------------------------------
int spiInit(void)
{
    struct spi_master *master;
    int ret;

    master = spi_busnum_to_master(0);
    if (!master) {
        pr_err("[AD9361][SPI INIT] No SPI master at bus 0\n");
        return -ENODEV;
    }

    spi_device_primary = spi_alloc_device(master);
    if (!spi_device_primary) {
        pr_err("[AD9361][SPI INIT] Failed to alloc SPI device\n");
        return -ENOMEM;
    }

    spi_device_primary->chip_select = 0;
    spi_device_primary->mode = SPI_MODE_0;
    spi_device_primary->bits_per_word = 8;
    spi_device_primary->max_speed_hz = 10000000;
    strlcpy(spi_device_primary->modalias, "ad9361", sizeof(spi_device_primary->modalias));

    ret = spi_setup(spi_device_primary);
    if (ret < 0) {
        pr_err("[AD9361][SPI INIT] spi_setup() failed: %d\n", ret);
        spi_dev_put(spi_device_primary);
        return ret;
    }

    pr_info("[AD9361][SPI INIT] SPI device initialized successfully\n");
    return 0;
}

// -----------------------------------------------------------------------------
// SPI Cleanup
// -----------------------------------------------------------------------------
void spiDestroy(void)
{
    if (spi_device_primary) {
        spi_dev_put(spi_device_primary);
        pr_info("[AD9361][SPI DESTROY] SPI device released\n");
    }
}

// -----------------------------------------------------------------------------
// Kernel Module Init
// -----------------------------------------------------------------------------
static int __init ad9361_spi_module_init(void)
{
    int ret;
    u16 reg_addr;
    u8 reg_val;

    pr_info("[AD9361][MODULE] Init AD9361 SPI module\n");

    ret = spiInit();
    if (ret < 0)
        return ret;

    msleep(50); // Wait after SPI init

    // Optional: write known values here before reading

    // Register dump
    for (reg_addr = 0x0000; reg_addr < AD9361_REG_COUNT; reg_addr++) {
        ret = ad9361_read_register(reg_addr, &reg_val);
        if (ret < 0) break;
        msleep(1);
    }

    pr_info("[AD9361][MODULE] Register dump complete\n");
    return 0;
}

// -----------------------------------------------------------------------------
// Kernel Module Exit
// -----------------------------------------------------------------------------
static void __exit ad9361_spi_module_exit(void)
{
    pr_info("[AD9361][MODULE] Exiting AD9361 SPI module\n");
    spiDestroy();
}

module_init(ad9361_spi_module_init);
module_exit(ad9361_spi_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice.Marek / OpenAI");
MODULE_DESCRIPTION("SPI Kernel Module for Reading AD9361 Registers");
