/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>  // For kmalloc and kfree
#include <linux/dma-mapping.h>  // For DMA support

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

static spiDmaData Dma[SPI_AMOUNT] =
{
    /* Input */
    [SPI_PRIMARY] =
    {
        .spiMessage = {},
        .spiTransfer = {0},
        .tx_dma = 0,
        .rx_dma = 0
    },

    /* Input */
    [SPI_SECONDARY] =
    {
        .spiMessage = {},
        .spiTransfer = {0},
        .tx_dma = 0,
        .rx_dma = 0
    },
};

static int spiBusInit(int bus, spiDeviceType device)
{
    struct spi_master *pMaster;
    int ret;

    pMaster = spi_busnum_to_master(bus);
    if (!pMaster)
    {
        printk(KERN_ERR "[INIT][SPI] SPI Master at BUS %d not found!\n", bus);
        return -ENODEV;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI Master at BUS %d Registered\n", bus);
    }

    Device[device].spiDevice = spi_alloc_device(pMaster);
    if (!Device[device].spiDevice)
    {
        printk(KERN_ERR "[INIT][SPI] SPI %d Failed to Allocate!\n", bus);
        return -ENOMEM;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI %d Allocated\n", bus);
    }

    /*!
     * The mode is set to 1 to pass the
     * High clock control signal to FPGA
     *
     * Only reqired when talking to FPGA
     */
    Device[device].spiDevice->chip_select = 0;
    Device[device].spiDevice->mode = SPI_MODE_1; /* For Kernel <=> FPGA Communication */
    Device[device].spiDevice->bits_per_word = 8;
    Device[device].spiDevice->max_speed_hz = 1000000;

    ret = spi_setup(Device[device].spiDevice);
    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][SPI] SPI %d device Failed to setup! ret[%d]\n", bus, ret);
        spi_dev_put(Device[device].spiDevice);
        return ret;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI %d device setup\n", bus);
    }

    return ret;
}

static int spiDmaInit(spiDeviceType spiDeviceEnum, charDeviceType charDeviceEnum)
{
    DataTransfer* pCharDeviceTransfer = getCharDeviceTransfer(charDeviceEnum);

    /* Allocate DMA buffers */
    Dma[spiDeviceEnum].tx_dma = dma_map_single(Device[spiDeviceEnum].spiDevice->controller->dev.parent, (void *)pCharDeviceTransfer->RxData, Device[spiDeviceEnum].spiLength, DMA_TO_DEVICE);
    Dma[spiDeviceEnum].rx_dma = dma_map_single(Device[spiDeviceEnum].spiDevice->controller->dev.parent, (void *)Device[spiDeviceEnum].spiRx, Device[spiDeviceEnum].spiLength, DMA_FROM_DEVICE);

    if(dma_mapping_error(Device[spiDeviceEnum].spiDevice->controller->dev.parent, Dma[spiDeviceEnum].tx_dma) ||
        dma_mapping_error(Device[spiDeviceEnum].spiDevice->controller->dev.parent, Dma[spiDeviceEnum].rx_dma))
    {
        printk(KERN_ERR "[INIT][SPI] DMA mapping failed for SPI %d\n", spiDeviceEnum);
        return -1;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] DMA mapping successful for SPI %d\n", spiDeviceEnum);
    }

    memset(&Dma[spiDeviceEnum].spiTransfer, 0, sizeof(Dma[spiDeviceEnum].spiTransfer));
    Dma[spiDeviceEnum].spiTransfer.tx_dma = Dma[spiDeviceEnum].tx_dma;
    Dma[spiDeviceEnum].spiTransfer.rx_dma = Dma[spiDeviceEnum].rx_dma;
    Dma[spiDeviceEnum].spiTransfer.len = Device[spiDeviceEnum].spiLength;
    Dma[spiDeviceEnum].spiTransfer.tx_buf = (void *)pCharDeviceTransfer->RxData;  /* Assign DMA buffer to tx_buf */
    Dma[spiDeviceEnum].spiTransfer.rx_buf = (void *)Device[spiDeviceEnum].spiRx;  /* Assign DMA buffer to rx_buf */
    Dma[spiDeviceEnum].spiTransfer.cs_change = 1;
    Dma[spiDeviceEnum].spiTransfer.speed_hz = Device[spiDeviceEnum].spiDevice->max_speed_hz;

    spi_message_init(&Dma[spiDeviceEnum].spiMessage);
    spi_message_add_tail(&Dma[spiDeviceEnum].spiTransfer, &Dma[spiDeviceEnum].spiMessage);

    return 0;
}

static int spiDmaDestroy(spiDeviceType spiDeviceEnum)
{
    /* Unmap DMA buffers */
    dma_unmap_single(Device[spiDeviceEnum].spiDevice->controller->dev.parent, Dma[spiDeviceEnum].tx_dma, Device[spiDeviceEnum].spiLength, DMA_TO_DEVICE);
    dma_unmap_single(Device[spiDeviceEnum].spiDevice->controller->dev.parent, Dma[spiDeviceEnum].rx_dma, Device[spiDeviceEnum].spiLength, DMA_FROM_DEVICE);

    return 0;
}

void transferFpgaInput(struct work_struct *work)
{
    int ret;
    int i;

    /* Initate DMA Controller to perform SPI transfer */
    ret = spi_sync(Device[SPI_PRIMARY].spiDevice, &Dma[SPI_PRIMARY].spiMessage);

    if (ret < 0)
    {
        printk(KERN_ERR "[CTRL][SPI] SPI transfer at signal From Char Device failed: %d\n", ret);
        return;
    }
    else
    {
        printk(KERN_INFO "[CTRL][SPI] Primary FPGA Transfer :: Signaled by transferFpgaInput over SPI.0\n");
    }

    for (i = 0; i < Device[SPI_PRIMARY].spiLength; ++i)
    {
        printk(KERN_INFO "[CTRL][SPI] Primary FPGA Transfer :: Byte[%d]: [Data] Tx[0x%02x] [Feedback] Rx[0x%02x]\n", i, Device[SPI_PRIMARY].spiRx[i], Device[SPI_PRIMARY].spiRx[i]);
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
    dma_addr_t tx_dma, rx_dma;
    int ret;
    int i;

    DataTransfer* kernelOutptData = getCharDeviceTransfer(DEVICE_OUTPUT);

    /* Allocate DMA buffers */
    tx_dma = dma_map_single(Device[SPI_SECONDARY].spiDevice->controller->dev.parent, (void *)Device[SPI_SECONDARY].spiTx, Device[SPI_SECONDARY].spiLength, DMA_TO_DEVICE);
    rx_dma = dma_map_single(Device[SPI_SECONDARY].spiDevice->controller->dev.parent, (void *)Device[SPI_SECONDARY].spiRx, Device[SPI_SECONDARY].spiLength, DMA_FROM_DEVICE);

    if (dma_mapping_error(Device[SPI_SECONDARY].spiDevice->controller->dev.parent, tx_dma) ||
        dma_mapping_error(Device[SPI_SECONDARY].spiDevice->controller->dev.parent, rx_dma))
    {
        printk(KERN_ERR "[CTRL][SPI] DMA mapping failed\n");
        return;
    }

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_dma = tx_dma;
    transfer.rx_dma = rx_dma;
    transfer.len = Device[SPI_SECONDARY].spiLength;
    transfer.tx_buf = (void *)Device[SPI_SECONDARY].spiTx;  /* Assign DMA buffer to tx_buf */
    transfer.rx_buf = (void *)Device[SPI_SECONDARY].spiRx;  /* Assign DMA buffer to rx_buf */
    transfer.cs_change = 1;
    transfer.speed_hz = Device[SPI_SECONDARY].spiDevice->max_speed_hz;

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    /* Initate DMA Controller to perform SPI transfer */
    ret = spi_sync(Device[SPI_SECONDARY].spiDevice, &msg);

    /* Unmap DMA buffers */
    dma_unmap_single(Device[SPI_SECONDARY].spiDevice->controller->dev.parent, tx_dma, Device[SPI_SECONDARY].spiLength, DMA_TO_DEVICE);
    dma_unmap_single(Device[SPI_SECONDARY].spiDevice->controller->dev.parent, rx_dma, Device[SPI_SECONDARY].spiLength, DMA_FROM_DEVICE);

    if (ret < 0)
    {
        printk(KERN_ERR "[CTRL][SPI] SPI transfer at interrupt From Fpga failed: %d\n", ret);
        return;
    }
    else
    {
        printk(KERN_INFO "[CTRL][SPI] Secondary FPGA Transfer :: Signaled by transferFpgaOutput over SPI.1\n");
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
    DataTransfer* kernelOutptData = getCharDeviceTransfer(DEVICE_OUTPUT);

    kernelOutptData->TxData[0] = 0xDE;
    kernelOutptData->TxData[1] = 0xAD;
    kernelOutptData->length = 2;

    unlockWaitMutex();
    unlockWatchdogMutex();
}

int spiInit(void)
{
    (void)spiBusInit(0, SPI_PRIMARY);
    (void)spiBusInit(1, SPI_SECONDARY);

    spiDmaInit(SPI_PRIMARY, DEVICE_INPUT);

    return 0;
}

void spiDestroy(void)
{
    spiDmaDestroy(SPI_PRIMARY);

    spi_dev_put(Device[SPI_PRIMARY].spiDevice);
    spi_dev_put(Device[SPI_SECONDARY].spiDevice);
    printk(KERN_INFO "[DESTROY][SPI] Destroy SPI Devices\n");
}
