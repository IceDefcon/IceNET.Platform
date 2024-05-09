/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#include "spiCtrl.h"
#include "isrCtrl.h"
#include "charDevice.h"

////////////////////////
//                    //
//                    //
//                    //
//   [SPI] COntrol    //
//                    //
//                    //
//                    //
////////////////////////

/*!
 * 
 * SPI0
 * ============================================
 * BBB P9_17 :: PULPLE   :: SPI0_CS0
 * BBB P9_18 :: BLUE     :: SPI0_D1
 * BBB P9_21 :: BROWN    :: SPI0_D0
 * BBB P9_22 :: BLACK    :: SPI0_SCLK
 * 
 * SPI1
 * ============================================
 * BBB P9_28 :: ORANGE   :: SPI1_CS0
 * BBB P9_30 :: YELOW    :: SPI1_D1
 * BBB P9_29 :: BLUE     :: SPI1_D0
 * BBB P9_31 :: GREEN    :: SPI1_SCLK
 * 
 */


static struct spi_device *spi_dev_primary;
static struct spi_device *spi_dev_secondary;

static volatile uint8_t spi_tx_at_interruptFromFpga[] = {0x81};
static volatile uint8_t spi_rx_at_interruptFromFpga[1];
static volatile uint8_t spi_tx_at_transferFromCharDevice[] = {0xAA};
static volatile uint8_t spi_rx_at_transferFromCharDevice[8];

int spiInit(void)
{

    struct spi_master *spi_master_primary;
    struct spi_master *spi_master_secondary;
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

    spi_master_secondary = spi_busnum_to_master(1);
    if (!spi_master_secondary) 
    {
        printk(KERN_ERR "[INIT][SPI] SPI Master at BUS 1 not found!\n");
        return -ENODEV;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI Master at BUS 1 Registered\n");
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

    spi_dev_secondary = spi_alloc_device(spi_master_secondary);
    if (!spi_dev_secondary) 
    {
        printk(KERN_ERR "[INIT][SPI] SPI1 Failed to Allocate!\n");
        return -ENOMEM;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI1 Allocated\n");
    }

    /*! 
     * The mode is set to 1 to pass the
     * High clock control signal to FPGA
     * 
     * Only reqired when talking to FPGA
     */
    spi_dev_primary->chip_select = 0;
    spi_dev_primary->mode = SPI_MODE_1; /* For Kernel <=> FPGA Communication */
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

    spi_dev_secondary->chip_select = 0;
    spi_dev_secondary->mode = SPI_MODE_1; /* For Kernel <=> FPGA Communication */
    spi_dev_secondary->bits_per_word = 8;
    spi_dev_secondary->max_speed_hz = 1000000;

    ret = spi_setup(spi_dev_secondary);
    if (ret < 0) {
        printk(KERN_ERR "[INIT][SPI] SPI1 device Failed to setup! ret[%d]\n", ret);
        spi_dev_put(spi_dev_secondary);
        return ret;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI1 device setup\n");
    }

}

void interruptFromFpga(struct work_struct *work)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = spi_tx_at_interruptFromFpga;
    transfer.rx_buf = spi_rx_at_interruptFromFpga;
    transfer.len = sizeof(spi_tx_at_interruptFromFpga);

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_dev_secondary, &msg);
    if (ret < 0) {
        printk(KERN_ERR "[CTRL][SPI] SPI transfer at interrupt From Fpga failed: %d\n", ret);
        return;
    }
    else
    {
        printk(KERN_INFO "[CTRL][SPI] Secondary FPGA Transfer :: Signaled by interruptFromFpga over SPI.1");
    }

    for (i = 0; i < sizeof(spi_rx_at_interruptFromFpga); ++i) {
        printk(KERN_INFO "[CTRL][SPI] Secondary FPGA Transfer :: Byte[%d]: [Preamble]Kernel.TX[0x%02x] [Data]Fpga.RX[0x%02x]\n", i, spi_tx_at_interruptFromFpga[i], spi_rx_at_interruptFromFpga[i]);
    }

    spi_rx_at_interruptFromFpga[0] = 0x00;

    /*!
     * 
     * 
     * 
     * Here we should receive data from
     * FPGA for kernel processing
     * 
     * 
     * 
     */
}

void transferFromCharDevice(struct work_struct *work)
{
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

    gpio_set_value(GPIO_PIN, 1); // Set pin high
    msleep(100); // Wait for 100 milliseconds
    gpio_set_value(GPIO_PIN, 0); // Set pin low

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
}

int spiDestroy(void)
{
    spi_dev_put(spi_dev_primary);
	spi_dev_put(spi_dev_secondary);
    printk(KERN_INFO "[DESTROY][SPI] Destroy SPI Devices\n");
}