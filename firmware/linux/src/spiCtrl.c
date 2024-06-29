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

static spiDeviceData Device[SPI_AMOUNT] =
{
    /* Input */
    [SPI_PRIMARY] =
    {
        .spiDevice = NULL,
        .spiTx = {0},
        .spiRx = {0},
        .spiLength = 4,
    },

    /* Output */
    [SPI_SECONDARY] =
    {
        .spiDevice = NULL,
        .spiTx = {0},
        .spiRx = {0},
        .spiLength = 1,
    },
};

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

    Device[SPI_PRIMARY].spiDevice = spi_alloc_device(spi_master_primary);
    if (!Device[SPI_PRIMARY].spiDevice)
    {
        printk(KERN_ERR "[INIT][SPI] SPI0 Failed to Allocate!\n");
        return -ENOMEM;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI0 Allocated\n");
    }

    Device[SPI_SECONDARY].spiDevice = spi_alloc_device(spi_master_secondary);
    if (!Device[SPI_SECONDARY].spiDevice)
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
    Device[SPI_PRIMARY].spiDevice->chip_select = 0;
    Device[SPI_PRIMARY].spiDevice->mode = SPI_MODE_1; /* For Kernel <=> FPGA Communication */
    Device[SPI_PRIMARY].spiDevice->bits_per_word = 8;
    Device[SPI_PRIMARY].spiDevice->max_speed_hz = 1000000;

    ret = spi_setup(Device[SPI_PRIMARY].spiDevice);
    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][SPI] SPI0 device Failed to setup! ret[%d]\n", ret);
        spi_dev_put(Device[SPI_PRIMARY].spiDevice);
        return ret;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI0 device setup\n");
    }

    Device[SPI_SECONDARY].spiDevice->chip_select = 0;
    Device[SPI_SECONDARY].spiDevice->mode = SPI_MODE_1; /* For Kernel <=> FPGA Communication */
    Device[SPI_SECONDARY].spiDevice->bits_per_word = 8;
    Device[SPI_SECONDARY].spiDevice->max_speed_hz = 1000000;

    ret = spi_setup(Device[SPI_SECONDARY].spiDevice);
    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][SPI] SPI1 device Failed to setup! ret[%d]\n", ret);
        spi_dev_put(Device[SPI_SECONDARY].spiDevice);
        return ret;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI1 device setup\n");
    }

    return 0;
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
    transfer.rx_buf = (void *)Device[SPI_PRIMARY].spiRx;
    transfer.len = Device[SPI_PRIMARY].spiLength;

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(Device[SPI_PRIMARY].spiDevice, &msg);
    if (ret < 0)
    {
        printk(KERN_ERR "[CTRL][SPI] SPI transfer at signal From Char Device failed: %d\n", ret);
        return;
    }
    else
    {
        printk(KERN_INFO "[CTRL][SPI] Primary FPGA Transfer :: Signaled by transferFpgaInput over SPI.0");
    }

    for (i = 0; i < Device[SPI_PRIMARY].spiLength; ++i)
    {
        printk(KERN_INFO "[CTRL][SPI] Primary FPGA Transfer :: Byte[%d]: [Data] Tx[0x%02x] [Feedback] Rx[0x%02x]\n", i, kernelOutptData->RxData[i], Device[SPI_PRIMARY].spiRx[i]);
    }

    if(0x18 != Device[SPI_PRIMARY].spiRx[0])
    {
        printk(KERN_ERR "[CTRL][SPI] No FPGA Preamble detected :: FPGA is Not Programed or Connected\n");
        unlockWaitMutex();
    }

    /* Clear the buffers */
    for (i = 0; i < Device[SPI_PRIMARY].spiLength; ++i)
    {
        Device[SPI_PRIMARY].spiRx[i] = 0x00;
        Device[SPI_PRIMARY].spiTx[i] = 0x00;
    }
}

void transferFpgaOutput(struct work_struct *work)
{
    struct spi_transfer transfer;
    struct spi_message msg;
    int ret;
    int i;

    DataTransfer* kernelOutptData = getKernelOutputTransfer();

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = (void *)Device[SPI_SECONDARY].spiTx;
    transfer.rx_buf = (void *)Device[SPI_SECONDARY].spiRx;
    transfer.len = Device[SPI_SECONDARY].spiLength;

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(Device[SPI_SECONDARY].spiDevice, &msg);
    if (ret < 0)
    {
        printk(KERN_ERR "[CTRL][SPI] SPI transfer at interrupt From Fpga failed: %d\n", ret);
        return;
    }
    else
    {
        printk(KERN_INFO "[CTRL][SPI] Secondary FPGA Transfer :: Signaled by transferFpgaOutput over SPI.1");
    }

    for (i = 0; i < Device[SPI_SECONDARY].spiLength; ++i)
    {
        printk(KERN_INFO "[CTRL][SPI] Secondary FPGA Transfer :: Byte[%d]: [Feedback] Tx[0x%02x] [Data] Rx[0x%02x]\n", i, Device[SPI_SECONDARY].spiTx[i], Device[SPI_SECONDARY].spiRx[i]);
    }

    /* Pass data from FPGA into Kernel Output Device */
    kernelOutptData->TxData[0] = (char)Device[SPI_SECONDARY].spiRx[0];
    kernelOutptData->length = 1;
    unlockWaitMutex();

    /* Clear the buffers */
    for (i = 0; i < Device[SPI_SECONDARY].spiLength; ++i)
    {
        Device[SPI_SECONDARY].spiRx[i] = 0x00;
        Device[SPI_SECONDARY].spiTx[i] = 0x00;
    }
}

void killApplication(struct work_struct *work)
{
    DataTransfer* kernelOutptData = getKernelOutputTransfer();

    kernelOutptData->TxData[0] = 0xDE;
    kernelOutptData->TxData[1] = 0xAD;
    kernelOutptData->length = 2;

    unlockWaitMutex();
}

void spiDestroy(void)
{
    spi_dev_put(Device[SPI_PRIMARY].spiDevice);
	spi_dev_put(Device[SPI_SECONDARY].spiDevice);
    printk(KERN_INFO "[DESTROY][SPI] Destroy SPI Devices\n");
}