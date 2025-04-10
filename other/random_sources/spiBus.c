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
//   [SPI] BMI160    //
//                    //
////////////////////////

#define BMI160_REG_CHIP_ID  0x00  // BMI160 Chip ID register
#define BMI160_EXPECTED_CHIP_ID 0xD1  // Expected Chip ID for BMI160

static struct spi_device *spi0_device;
static uint8_t spiTx[2] = {0};
static uint8_t spiRx[2] = {0};

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
    spi0_device->mode = SPI_MODE_1;       // SPI Mode 0 (CPOL=0, CPHA=0) for BMI160
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

void bmi160_read_chip_id(void)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    spiTx[0] = BMI160_REG_CHIP_ID | 0x80; // Set MSB for read operation
    spiTx[1] = 0x00;

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = (void *)spiTx;
    transfer.rx_buf = (void *)spiRx;
    transfer.len = 2; // Send address, receive data

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi0_device, &msg);
    if (ret < 0)
    {
        printk(KERN_ERR "[BMI160][SPI] Failed to read Chip ID, error %d\n", ret);
    }

    for (i = 0; i < transfer.len; i++)
    {
        printk(KERN_INFO "[BMI160][SPI] Byte [%d] 0x%02X \n", i, spiRx[i]);
    }
}

static int __init spi_ctrl_init(void)
{
    int ret;

    printk(KERN_INFO "[INIT][SPI] BMI160 SPI Control Module Loading...\n");

    ret = spiInit();
    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][SPI] SPI initialization failed!\n");
        return ret;
    }

    bmi160_read_chip_id();

    return 0;
}

static void __exit spi_ctrl_exit(void)
{
    printk(KERN_INFO "[EXIT][SPI] BMI160 SPI Control Module Unloading...\n");

    if (spi0_device)
    {
        spi_dev_put(spi0_device);
        printk(KERN_INFO "[EXIT][SPI] SPI0 device released.\n");
    }
}

module_init(spi_ctrl_init);
module_exit(spi_ctrl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice.Marek");
MODULE_DESCRIPTION("BMI160 SPI Driver");
