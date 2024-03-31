/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <linux/module.h>
#include <linux/kernel.h>

#include "spiFpga.h"

//////////////////////
//                  //
//                  //
//                  //
//   [SPI] Comms    //
//                  //
//                  //
//                  //
//////////////////////////////////////////
//                                      //
//                                      //
// BBB P9_17 :: PULPLE   :: SPI0_CS0    //
// BBB P9_18 :: BLUE     :: SPI0_D1     //
// BBB P9_21 :: BROWN    :: SPI0_D0     //
// BBB P9_22 :: BLACK    :: SPI0_SCLK   //
//                                      //
// BBB P9_28 :: YELOW    :: SPI1_CS0    //
// BBB P9_30 :: GREEN    :: SPI1_D1     //
// BBB P9_29 :: RED      :: SPI1_D0     //
// BBB P9_31 :: ORANGE   :: SPI1_SCLK   //
//                                      //
//                                      //
//////////////////////////////////////////


struct spi_device *spi_dev;

static volatile uint8_t tx_kernel[] = {0x81};
static volatile uint8_t rx_kernel[1];

static volatile uint8_t tx_fpga[] = {0xC3};
static volatile uint8_t rx_fpga[1];

int spiInit(void)
{

    struct spi_master *spi_master0;
    int ret;

    spi_master0 = spi_busnum_to_master(0);
    if (!spi_master0) {
        printk(KERN_ERR "[FPGA][SPI] SPI master for SPI0 not found\n");
        return -ENODEV;
    }

    // Prepare the SPI devices
    spi_dev = spi_alloc_device(spi_master0);
    if (!spi_dev) {
        printk(KERN_ERR "[FPGA][SPI] Failed to allocate SPI device for SPI0\n");
        return -ENOMEM;
    }

    /*! 
     * The mode is set to 1 to pass the
     * High clock control signal to FPGA
     */
    spi_dev->chip_select = 0;
    spi_dev->mode = SPI_MODE_1;
    spi_dev->bits_per_word = 8;
    spi_dev->max_speed_hz = 1000000;

    ret = spi_setup(spi_dev);
    if (ret < 0) {
        printk(KERN_ERR "[FPGA][SPI] Failed to setup SPI device: %d\n", ret);
        spi_dev_put(spi_dev);
        return ret;
    }
}

void spiKernelExecute(struct work_struct *work)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = tx_kernel;
    transfer.rx_buf = rx_kernel;
    transfer.len = sizeof(tx_kernel);

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_dev, &msg);
    if (ret < 0) {
        printk(KERN_ERR "[FPGA][SPI] SPI transfer for Kernel failed: %d\n", ret);
        return;
    }

    printk(KERN_INFO "[FPGA][SPI] Command Data for Kernel Processing");
    for (i = 0; i < sizeof(rx_kernel); ++i) {
        printk(KERN_INFO "[FPGA][SPI] Byte %d: 0x%02x\n", i, rx_kernel[i]);
    }

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

void spiFpgaExecute(struct work_struct *work)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = tx_fpga;
    transfer.rx_buf = rx_fpga;
    transfer.len = sizeof(tx_fpga);

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_dev, &msg);
    if (ret < 0) {
        printk(KERN_ERR "[FPGA][SPI] SPI transfer for FPGA failed: %d\n", ret);
        return;
    }

    printk(KERN_INFO "[FPGA][SPI] Feedback Data for Kernel Processing");
    for (i = 0; i < sizeof(rx_fpga); ++i) {
        printk(KERN_INFO "[FPGA][SPI] Byte %d: 0x%02x\n", i, rx_fpga[i]);
    }

    /*!
     * 
     * 
     * 
     * Here we should receive movement
     * feedback data for kernel processing
     * 
     * 
     * 
     */

}

int spiDestroy(void)
{
	spi_dev_put(spi_dev);
    printk(KERN_INFO "[FPGA][SPI] Exit\n");
}