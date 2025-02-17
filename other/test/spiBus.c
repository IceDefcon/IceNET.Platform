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

////////////////////////
//                    //
//                    //
//                    //
//   [SPI] Control    //
//                    //
//                    //
//                    //
////////////////////////

static struct spi_device *spi0_device;

static struct spi_transfer transfer;
static uint8_t spiTx[10] = {0};
static uint8_t spiRx[10] = {0};
static int spiLength = 1;

int spiInit(void)
{
    struct spi_master *spi_master;
    int ret;

    spi_master = spi_busnum_to_master(0);
    if (!spi_master)
    {
        printk(KERN_ERR "[INIT][SPI] SPI Master at BUS 0 not found!\n");
        return -ENODEV;
    }

    printk(KERN_INFO "[INIT][SPI] SPI Master at BUS 0 Registered\n");

    spi0_device = spi_alloc_device(spi_master);
    if (!spi0_device)
    {
        printk(KERN_ERR "[INIT][SPI] Failed to allocate SPI0 device!\n");
        return -ENOMEM;
    }

    spi0_device->chip_select = 0;
    spi0_device->mode = SPI_MODE_0;       // SPI Mode 0 (CPOL=0, CPHA=0) :: For nRF905
    spi0_device->bits_per_word = 8;       // 8 bits per word
    spi0_device->max_speed_hz = 5000000;  // 5 MHz clock speed

    ret = spi_setup(spi0_device);
    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][SPI] SPI0 device setup failed! ret[%d]\n", ret);
        spi_dev_put(spi0_device);
        return ret;
    }

    printk(KERN_INFO "[INIT][SPI] SPI0 device setup complete.\n");
    return 0;
}

u8 nrf905_read_register(u8 reg)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;

    // Construct read command: 0001 AAAA
    spiTx[0] = (0x10 | (reg & 0x0F)); // Read command (0x10) + 4-bit address
    spiTx[1] = 0x00;  // Dummy byte to clock out data

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = (void *)spiTx;
    transfer.rx_buf = (void *)spiRx;
    transfer.len = 2; // Sending command, receiving 1-byte response

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi0_device, &msg);
    if (ret < 0)
    {
        printk(KERN_ERR "[NRF905][SPI] Failed to read register 0x%02X, error %d\n", reg, ret);
        return 0xFF; // Invalid value on error
    }

    printk(KERN_INFO "[NRF905][SPI] Register 0x%02X = 0x%02X\n", reg, spiRx[1]);

    return spiRx[1];
}


void nrf905_read_multiple_registers(u8 start_reg, u8 num_bytes)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    spiTx[0] = (0x10 | (start_reg & 0x0F)); // Read command + start register

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = (void *)spiTx;
    transfer.rx_buf = (void *)spiRx;
    transfer.len = num_bytes + 1; // Command + response bytes

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi0_device, &msg);
    if (ret < 0)
    {
        printk(KERN_ERR "[NRF905][SPI] Failed to read registers from 0x%02X, error %d\n", start_reg, ret);
        return;
    }

    for (i = 1; i <= num_bytes; i++) // Skip first byte (command)
    {
        printk(KERN_INFO "[NRF905][SPI] Register 0x%02X = 0x%02X\n", start_reg + i - 1, spiRx[i]);
    }
}

int readConfigRegister(void)
{
    struct spi_message msg;
    int ret;
    int i;

    spiTx[0] = 0x10; /* Read command for CONFIG register */
    memset(spiRx, 0, sizeof(spiRx));

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = spiTx;
    transfer.rx_buf = spiRx;
    transfer.len = spiLength;

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi0_device, &msg);
    if (ret < 0)
    {
        printk(KERN_ERR "[CTRL][SPI] SPI0 transfer failed: %d\n", ret);
        return ret;
    }

    printk(KERN_INFO "[CTRL][SPI] Read CONFIG Register Successful.\n");
    for (i = 0; i < spiLength; ++i)
    {
        printk(KERN_INFO "[CTRL][SPI] Byte[%d]: Tx[0x%02x] Rx[0x%02x]\n", i, spiTx[i], spiRx[i]);
    }

    return 0;
}

void spiDestroy(void)
{
    if (spi0_device)
    {
        spi_dev_put(spi0_device);
        printk(KERN_INFO "[DESTROY][SPI] SPI0 device destroyed.\n");
    }
}

static int __init spi_ctrl_init(void)
{
    int ret;

    printk(KERN_INFO "[INIT][SPI] SPI Control Module Loading...\n");

    ret = spiInit();
    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][SPI] SPI initialization failed!\n");
        return ret;
    }

    // readConfigRegister();

    // nrf905_read_register(0x02);

    nrf905_read_multiple_registers(0x00, 0x0A);

    return 0;
}

static void __exit spi_ctrl_exit(void)
{
    printk(KERN_INFO "[EXIT][SPI] SPI Control Module Unloading...\n");

    // Cleanup the SPI0 device
    spiDestroy();
}

module_init(spi_ctrl_init);
module_exit(spi_ctrl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice.Marek");
MODULE_DESCRIPTION("Testing :: nRF905");
