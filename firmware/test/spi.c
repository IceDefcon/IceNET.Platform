#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/kernel.h>
#include <linux/init.h>

static struct spi_device *spi_dev_primary;

static volatile uint8_t spi_tx_at_transferFromCharDevice[] = {0xAA};
static volatile uint8_t spi_rx_at_transferFromCharDevice[8];

static int __init spi_module_init(void) {
 
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

    spi_dev_primary = spi_alloc_device(spi_master_primary);
    if (!spi_dev_primary) 
    {
        printk(KERN_ERR "[INIT][SPI] SPI0 Failed to Allocate!\n");
        return -ENOMEM;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI0 Allocated\n");
    }

    /*! 
     * The mode is set to 1 to pass the
     * High clock control signal to FPGA
     * 
     * Only reqired when talking to FPGA
     */
    spi_dev_primary->chip_select = 0;
    spi_dev_primary->mode = SPI_MODE_0;
    spi_dev_primary->bits_per_word = 8;
    spi_dev_primary->max_speed_hz = 1000000;

    ret = spi_setup(spi_dev_primary);
    if (ret < 0) {
        printk(KERN_ERR "[INIT][SPI] SPI0 device Failed to setup! ret[%d]\n", ret);
        spi_dev_put(spi_dev_primary);
        return ret;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI0 device setup\n");
    }

    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    DataTransfer* fpgaData = charDevice_getRxData();

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = fpgaData->RxData;
    transfer.rx_buf = spi_rx_at_transferFromCharDevice;
    transfer.len = fpgaData->length;

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_dev_primary, &msg);
    if (ret < 0) 
    {
        printk(KERN_ERR "[CTRL][SPI] SPI transfer at signal From Char Device failed: %d\n", ret);
        return;
    }
    else
    {
        printk(KERN_INFO "[CTRL][SPI] Primary FPGA Transfer :: Signaled by transferFromCharDevice over SPI.0");
    }

    for (i = 0; i < fpgaData->length; ++i) 
    {
        printk(KERN_INFO "[CTRL][SPI] Primary FPGA Transfer :: Byte[%d]: [Data]Kernel.TX[0x%02x] [Preamble]Fpga.RX[0x%02x]\n", i, fpgaData->RxData[i], spi_rx_at_transferFromCharDevice[i]);
    }

    /*!
     * 
     * Here we should process 
     * feedback from FPGA
     * 
     * Then Clear the buffer
     * 
     */

    for (i = 0; i < fpgaData->length; ++i) 
    {
        spi_rx_at_transferFromCharDevice[i] = 0x00;
    }

    return 0;
}

static void __exit spi_module_exit(void) 
{
    spi_dev_put(spi_dev_primary);
}

module_init(spi_module_init);
module_exit(spi_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("SPI Read Kernel Module");
MODULE_VERSION("0.1");
