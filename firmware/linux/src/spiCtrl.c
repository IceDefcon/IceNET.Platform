/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>  // For kmalloc and kfree

#include "spiCtrl.h"
#include "charDevice.h"
#include "stateMachine.h"
#include "types.h"

////////////////////////
//                    //
//                    //
//                    //
//   [SPI] Control    //
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

typedef struct
{
    struct spi_device *primarySpi;
    struct spi_device *secondarySpi;

}spiCtrl;



static struct spi_device *spi_dev_primary;
static struct spi_device *spi_dev_secondary;

static volatile uint8_t spi_tx_at_transferFpgaInput[] = {0xAA};
static volatile uint8_t spi_rx_at_transferFpgaInput[8];
static volatile uint8_t spi_tx_at_transferFpgaOutput[] = {0x81};
static volatile uint8_t spi_rx_at_transferFpgaOutput[1];

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

    return 0;
}

void transferFpgaOutput(struct work_struct *work)
{
    DataTransfer* kernelOutptData;
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = (void *)spi_tx_at_transferFpgaOutput;
    transfer.rx_buf = (void *)spi_rx_at_transferFpgaOutput;
    transfer.len = sizeof(spi_tx_at_transferFpgaOutput);

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_dev_secondary, &msg);
    if (ret < 0) {
        printk(KERN_ERR "[CTRL][SPI] SPI transfer at interrupt From Fpga failed: %d\n", ret);
        return;
    }
    else
    {
        printk(KERN_INFO "[CTRL][SPI] Secondary FPGA Transfer :: Signaled by transferFpgaOutput over SPI.1");
    }

    for (i = 0; i < sizeof(spi_rx_at_transferFpgaOutput); ++i) {
        printk(KERN_INFO "[CTRL][SPI] Secondary FPGA Transfer :: Byte[%d]: [Feedback]Kernel.TX[0x%02x] [Data]Fpga.RX[0x%02x]\n", i, spi_tx_at_transferFpgaOutput[i], spi_rx_at_transferFpgaOutput[i]);
    }

    kernelOutptData = getKernelOutputTransfer();
    kernelOutptData->TxData[0] = (char)spi_rx_at_transferFpgaOutput[0];
    kernelOutptData->length = 1;
    // setStateMachine(FEEDBACK);
    unlockWaitMutex();
}

void transferFpgaInput(struct work_struct *work)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    DataTransfer* kernelOutptData = getKernelInputTransfer();

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = (void *)kernelOutptData->RxData;
    transfer.rx_buf = (void *)spi_rx_at_transferFpgaInput;
    transfer.len = kernelOutptData->length;

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
        printk(KERN_INFO "[CTRL][SPI] Primary FPGA Transfer :: Signaled by transferFpgaInput over SPI.0");
    }

    for (i = 0; i < kernelOutptData->length; ++i)
    {
        printk(KERN_INFO "[CTRL][SPI] Primary FPGA Transfer :: Byte[%d]: [Data]Kernel.TX[0x%02x] [Feedback]Fpga.RX[0x%02x]\n", i, kernelOutptData->RxData[i], spi_rx_at_transferFpgaInput[i]);
    }

    if(0x18 != spi_rx_at_transferFpgaInput[0])
    {
        printk(KERN_ERR "[CTRL][SPI] No FPGA Preamble detected :: FPGA is Not Programed or Connected\n");
        unlockWaitMutex();
    }

    /**
     * 
     * Here we should process 
     * additional feedback 
     * from FPGA
     * 
     * Then Clear the buffer
     * 
     */

    for (i = 0; i < kernelOutptData->length; ++i)
    {
        spi_rx_at_transferFpgaInput[i] = 0x00;
    }
}

void killApplication(struct work_struct *work)
{
    DataTransfer* kernelOutptData;
    kernelOutptData = getKernelOutputTransfer();
    kernelOutptData->TxData[0] = 0xDE;
    kernelOutptData->TxData[1] = 0xAD;
    kernelOutptData->length = 2;

    unlockWaitMutex();
}

void spiDestroy(void)
{
    spi_dev_put(spi_dev_primary);
	spi_dev_put(spi_dev_secondary);
    printk(KERN_INFO "[DESTROY][SPI] Destroy SPI Devices\n");
}