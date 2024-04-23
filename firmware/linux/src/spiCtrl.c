/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <linux/module.h>
#include <linux/kernel.h>

#include "spiCtrl.h"

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
 * BBB P9_28 :: ORANGE   :: SPI1_CS0  :: GREEN
 * BBB P9_30 :: YELOW    :: SPI1_D1   :: BLUE
 * BBB P9_29 :: BLUE     :: SPI1_D0   :: PURPLE
 * BBB P9_31 :: GREEN    :: SPI1_SCLK :: GREY
 * 
 */


static struct spi_device *spi_dev_main;
static struct spi_device *spi_dev_second;

static volatile uint8_t spi_tx_at_interruptFromFpga[] = {0x81};
static volatile uint8_t spi_rx_at_interruptFromFpga[1];
static volatile uint8_t spi_tx_at_signalFromCharDevice[] = {0xC3};
static volatile uint8_t spi_rx_at_signalFromCharDevice[1];
static volatile uint8_t spi_tx_at_testFromCharDevice[] = {0x00}; /* ID Register of the BMI160 chip */
static volatile uint8_t spi_rx_at_testFromCharDevice[1];

int spiInit(void)
{

    struct spi_master *spi_master0;
    struct spi_master *spi_master1;
    int ret;

    spi_master0 = spi_busnum_to_master(0);
    if (!spi_master0) {
        printk(KERN_ERR "[CONFIG][SPI] SPI Master at BUS 0 not found!\n");
        return -ENODEV;
    }
    else
    {
        printk(KERN_ERR "[CONFIG][SPI] SPI Master at BUS 0 Registered\n");
    }

    spi_master1 = spi_busnum_to_master(1);
    if (!spi_master1) {
        printk(KERN_ERR "[CONFIG][SPI] SPI Master at BUS 1 not found!\n");
        return -ENODEV;
    }
    else
    {
        printk(KERN_ERR "[CONFIG][SPI] SPI Master at BUS 1 Registered\n");
    }

    spi_dev_main = spi_alloc_device(spi_master0);
    if (!spi_dev_main) {
        printk(KERN_ERR "[CONFIG][SPI] SPI0 Failed to Allocate!\n");
        return -ENOMEM;
    }
    else
    {
        printk(KERN_ERR "[CONFIG][SPI] SPI0 Allocated\n");
    }

    spi_dev_second = spi_alloc_device(spi_master1);
    if (!spi_dev_second) {
        printk(KERN_ERR "[CONFIG][SPI] SPI1 Failed to Allocate!\n");
        return -ENOMEM;
    }
    else
    {
        printk(KERN_ERR "[CONFIG][SPI] SPI1 Allocated\n");
    }

    /*! 
     * The mode is set to 1 to pass the
     * High clock control signal to FPGA
     * 
     * Only reqired when talking to FPGA
     */
    spi_dev_main->chip_select = 0;
    spi_dev_main->mode = SPI_MODE_1;
    spi_dev_main->bits_per_word = 8;
    spi_dev_main->max_speed_hz = 1000000;

    ret = spi_setup(spi_dev_main);
    if (ret < 0) {
        printk(KERN_ERR "[CONFIG][SPI] SPI0 device Failed to setup! ret[%d]\n", ret);
        spi_dev_put(spi_dev_main);
        return ret;
    }
    else
    {
        printk(KERN_ERR "[CONFIG][SPI] SPI0 device setup\n");
    }

    spi_dev_second->chip_select = 0;
    spi_dev_second->mode = SPI_MODE_1; /* For Kernel <=> FPGA Communication */
    spi_dev_second->bits_per_word = 8;
    spi_dev_second->max_speed_hz = 1000000;

    ret = spi_setup(spi_dev_second);
    if (ret < 0) {
        printk(KERN_ERR "[CONFIG][SPI] SPI1 device Failed to setup! ret[%d]\n", ret);
        spi_dev_put(spi_dev_second);
        return ret;
    }
    else
    {
        printk(KERN_ERR "[CONFIG][SPI] SPI1 device setup\n");
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

    ret = spi_sync(spi_dev_main, &msg);
    if (ret < 0) {
        printk(KERN_ERR "[CTRL][SPI] SPI transfer at interrupt From Fpga failed: %d\n", ret);
        return;
    }

    printk(KERN_INFO "[CTRL][SPI] Data from FPGA ---==[ FPGA Button :: Read from I2C Gyroscope driven in FPGA ]==---");
    for (i = 0; i < sizeof(spi_rx_at_interruptFromFpga); ++i) {
        printk(KERN_INFO "[CTRL][SPI] Byte %d: 0x%02x\n", i, spi_rx_at_interruptFromFpga[i]);
    }

    for (i = 0; i < sizeof(spi_tx_at_interruptFromFpga); ++i) {
        printk(KERN_INFO "[CTRL][SPI] Write to FPGA %d: 0x%02x\n ---==[ GPIO Interrupt ]==---", i, spi_tx_at_interruptFromFpga[i]);
    }
    for (i = 0; i < sizeof(spi_rx_at_interruptFromFpga); ++i) {
        printk(KERN_INFO "[CTRL][SPI] Read from FPGA %d: 0x%02x\n ---==[ GPIO Interrupt ]==---", i, spi_rx_at_interruptFromFpga[i]);
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

void signalFromCharDevice(struct work_struct *work)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = spi_tx_at_signalFromCharDevice;
    transfer.rx_buf = spi_rx_at_signalFromCharDevice;
    transfer.len = sizeof(spi_tx_at_signalFromCharDevice);

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_dev_main, &msg);
    if (ret < 0) {
        printk(KERN_ERR "[CTRL][SPI] SPI transfer at signal From Char Device failed: %d\n", ret);
        return;
    }

    for (i = 0; i < sizeof(spi_tx_at_signalFromCharDevice); ++i) {
        printk(KERN_INFO "[CTRL][SPI] Write to FPGA %d: 0x%02x\n ---==[ C Device Signal SPI.0 ]==---", i, spi_tx_at_signalFromCharDevice[i]);
    }
    for (i = 0; i < sizeof(spi_rx_at_signalFromCharDevice); ++i) {
        printk(KERN_INFO "[CTRL][SPI] Read from FPGA %d: 0x%02x\n ---==[ C Device Signal SPI.0 ]==---", i, spi_rx_at_signalFromCharDevice[i]);
    }

    spi_rx_at_signalFromCharDevice[0] = 0x00;

    /*!
     * 
     * Here we should process 
     * feedback from FPGA
     * 
     */

}

void testFromCharDevice(struct work_struct *work)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = spi_tx_at_testFromCharDevice;
    transfer.rx_buf = spi_rx_at_testFromCharDevice;
    transfer.len = sizeof(spi_tx_at_testFromCharDevice);

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_dev_second, &msg);
    if (ret < 0) {
        printk(KERN_ERR "[TEST][SPI] SPI transfer for FPGA failed: %d\n", ret);
        return;
    }

    printk(KERN_INFO "[TEST][SPI] Data from FPGA ---==[ SPI1 :: Constant from FPGA Reqister ]==---");
    for (i = 0; i < sizeof(spi_rx_at_testFromCharDevice); ++i) {
        printk(KERN_INFO "[TEST][SPI] Byte %d: 0x%02x\n", i, spi_rx_at_testFromCharDevice[i]);
    }

    for (i = 0; i < sizeof(spi_tx_at_testFromCharDevice); ++i) {
        printk(KERN_INFO "[CTRL][SPI] Write to FPGA %d: 0x%02x\n ---==[ C Device Signal SPI.1 ]==---", i, spi_tx_at_testFromCharDevice[i]);
    }
    for (i = 0; i < sizeof(spi_rx_at_testFromCharDevice); ++i) {
        printk(KERN_INFO "[CTRL][SPI] Read from FPGA %d: 0x%02x\n ---==[ C Device Signal SPI.1 ]==---", i, spi_rx_at_testFromCharDevice[i]);
    }

    spi_rx_at_testFromCharDevice[0] = 0x00;

    /*!
     * 
     * Here we should process 
     * feedback from FPGA
     * 
     */

}

int spiDestroy(void)
{
    spi_dev_put(spi_dev_main);
	spi_dev_put(spi_dev_second);
    printk(KERN_INFO "[DESTROY][SPI] Destroy SPI Devices\n");
}