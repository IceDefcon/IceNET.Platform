/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>  // For kmalloc and kfree
#include <linux/dma-mapping.h>  // For DMA support

#include "stateMachine.h"
#include "charDevice.h"
#include "spiDmaCtrl.h"
#include "ramAxis.h"
#include "types.h"
#include "config.h"

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
    [SPI_PRIMARY] = /* SPI 0 */
    {
        .spiDevice = NULL,
        .spiLength = 0,

        .Dma =
        {
            .spiMessage = {},
            .spiTransfer = {0},
            .tx_dma = 0,
            .rx_dma = 0
        }
    },

    [SPI_SECONDARY] = /* SPI 1 */
    {
        .spiDevice = NULL,
        .spiLength = 0,

        .Dma =
        {
            .spiMessage = {},
            .spiTransfer = {0},
            .tx_dma = 0,
            .rx_dma = 0
        }
    },
};

static int spiBusInit(spiBusType spiBusEnum, spiDeviceType spiDeviceEnum)
{
    struct spi_master *pMaster;
    int ret;

    pMaster = spi_busnum_to_master(spiBusEnum);
    if (!pMaster)
    {
        printk(KERN_ERR "[INIT][SPI] SPI %d Master BUS not found!\n", spiBusEnum);
        return -ENODEV;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI %d Master BUS Registered\n", spiBusEnum);
    }

    Device[spiDeviceEnum].spiDevice = spi_alloc_device(pMaster);
    if (!Device[spiDeviceEnum].spiDevice)
    {
        printk(KERN_ERR "[INIT][SPI] SPI %d Failed to Allocate!\n", spiBusEnum);
        return -ENOMEM;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI %d Allocated\n", spiBusEnum);
    }

    /*!
     * The mode is set to 1 to pass the
     * High clock control signal to FPGA
     *
     * Only reqired when talking to FPGA
     */
    Device[spiDeviceEnum].spiDevice->chip_select = 0;
    Device[spiDeviceEnum].spiDevice->mode = SPI_MODE_1; /* For Kernel <=> FPGA Communication */
    Device[spiDeviceEnum].spiDevice->bits_per_word = 8;
    Device[spiDeviceEnum].spiDevice->max_speed_hz = 1000000;

    ret = spi_setup(Device[spiDeviceEnum].spiDevice);
    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][SPI] SPI %d device Failed to setup! ret[%d]\n", spiBusEnum, ret);
        spi_dev_put(Device[spiDeviceEnum].spiDevice);
        return ret;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] SPI %d device setup\n", spiBusEnum);
    }

    return ret;
}


 /*************************************************************************************************************************************************
 *
 *                                                          -----===[ FORWARD ]===-----
 *
 **************************************************************************************************************************************************
 *
 * APPLICATION -----------> [0] DEVICE_COMMANDER ---------------------------------------------------------------------------------------------
 *                           |                                                                                                               |
 *                           V                                                                                                               |
 *                          [0] SPI_PRIMARY -------------------->                                                                            |
 *                           |                                  |                                                                            |
 *                           V                                  |                                                                            |
 * FPGA <------------------ [0] DMA_IN                          |                                                                            |
 *                                                              V                                                                            V
 * Device[SPI_PRIMARY].Dma.tx_dma = dma_map_single(Device[SPI_PRIMARY].spiDevice->controller->dev.parent, (void *)getCharDeviceTransfer(DEVICE_COMMANDER)->RxData, Device[SPI_PRIMARY].spiLength, DMA_TO_DEVICE);
 * Device[SPI_PRIMARY].Dma.rx_dma = dma_map_single(Device[SPI_PRIMARY].spiDevice->controller->dev.parent, (void *)Device[SPI_PRIMARY].spiRx, Device[SPI_PRIMARY].spiLength, DMA_FROM_DEVICE);
 *
 */



 /*************************************************************************************************************************************************
 *
 *                                                          -----===[ FEEDBACK ]===-----
 *
 **************************************************************************************************************************************************
 *
 * APPLICATION <----------- [1] DEVICE_COMMANDER <-------------------------------------------------------------------------------------------------
 *                           Λ                                                                                                                    Λ
 *                           |                                                                                                                    |
 *                          [1] SPI_SECONDARY <--------------------                                                                               |
 *                           Λ                                    Λ                                                                               |
 *                           |                                    |                                                                               |
 * FPGA ------------------> [1] DMA_OUT                           |                                                                               |
 *                                                                |                                                                               |
 * Device[SPI_SECONDARY].Dma.tx_dma = dma_map_single(Device[SPI_SECONDARY].spiDevice->controller->dev.parent, (void *)getCharDeviceTransfer(DEVICE_COMMANDER)->RxData, Device[SPI_SECONDARY].spiLength, DMA_TO_DEVICE);
 * Device[SPI_SECONDARY].Dma.rx_dma = dma_map_single(Device[SPI_SECONDARY].spiDevice->controller->dev.parent, (void *)Device[SPI_SECONDARY].spiRx, Device[SPI_SECONDARY].spiLength, DMA_FROM_DEVICE);
 *
 */

static int spiDmaInit(spiDeviceType spiDeviceEnum, dmaControlType dmaControl, bool fpgaConfig)
{
    DmaTransferType* dmaTransfer;

    if(true == fpgaConfig)
    {
        dmaTransfer = getDmaTransfer();

        if(SPI_PRIMARY == spiDeviceEnum)
        {
            printk(KERN_ERR "[INIT][SPI] SPI/DMA FPGA Config\n");
            Device[spiDeviceEnum].spiLength = getConfigBytesAmount();
        }
        else if(SPI_SECONDARY == spiDeviceEnum)
        {
            printk(KERN_ERR "[INIT][SPI] SPI/DMA Feedback\n");
            Device[spiDeviceEnum].spiLength = FEEDBACK_DMA_TRANSFER_SIZE;
        }
        else
        {
            printk(KERN_ERR "[INIT][SPI] Wrong SPI Device\n");
        }
    }
    else if(false == fpgaConfig)
    {
        dmaTransfer = getCharDeviceTransfer(DEVICE_COMMANDER);

        if(SPI_PRIMARY == spiDeviceEnum)
        {
            printk(KERN_ERR "[INIT][SPI] SPI/DMA Server Config\n");
            Device[spiDeviceEnum].spiLength = MANUAL_DMA_TRANSFER_SIZE;
        }
        else if(SPI_SECONDARY == spiDeviceEnum)
        {
            printk(KERN_ERR "[INIT][SPI] SPI/DMA Feedback\n");
            Device[spiDeviceEnum].spiLength = FEEDBACK_DMA_TRANSFER_SIZE;
        }
        else
        {
            printk(KERN_ERR "[INIT][SPI] Wrong SPI Device\n");
        }
    }

    /* Allocate DMA buffers */
    Device[spiDeviceEnum].Dma.tx_dma = dma_map_single(Device[spiDeviceEnum].spiDevice->controller->dev.parent, (void *)dmaTransfer->RxData, Device[spiDeviceEnum].spiLength, DMA_TO_DEVICE);
    Device[spiDeviceEnum].Dma.rx_dma = dma_map_single(Device[spiDeviceEnum].spiDevice->controller->dev.parent, (void *)dmaTransfer->TxData, Device[spiDeviceEnum].spiLength, DMA_FROM_DEVICE);

    if(dma_mapping_error(Device[spiDeviceEnum].spiDevice->controller->dev.parent, Device[spiDeviceEnum].Dma.tx_dma) ||
        dma_mapping_error(Device[spiDeviceEnum].spiDevice->controller->dev.parent, Device[spiDeviceEnum].Dma.rx_dma))
    {
        printk(KERN_ERR "[INIT][SPI] DMA mapping failed for SPI %d\n", spiDeviceEnum);
        return -1;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] DMA mapping successful for SPI %d\n", spiDeviceEnum);
    }

    memset(&Device[spiDeviceEnum].Dma.spiTransfer, 0, sizeof(Device[spiDeviceEnum].Dma.spiTransfer));
    Device[spiDeviceEnum].Dma.spiTransfer.tx_dma = Device[spiDeviceEnum].Dma.tx_dma;
    Device[spiDeviceEnum].Dma.spiTransfer.rx_dma = Device[spiDeviceEnum].Dma.rx_dma;
    Device[spiDeviceEnum].Dma.spiTransfer.len = Device[spiDeviceEnum].spiLength;

    if(DMA_IN == dmaControl)
    {
        Device[spiDeviceEnum].Dma.spiTransfer.tx_buf = (void *)dmaTransfer->RxData;
        Device[spiDeviceEnum].Dma.spiTransfer.rx_buf = (void *)dmaTransfer->TxData;
    }
    else if(DMA_OUT == dmaControl)
    {
        Device[spiDeviceEnum].Dma.spiTransfer.tx_buf = (void *)dmaTransfer->RxData;
        Device[spiDeviceEnum].Dma.spiTransfer.rx_buf = (void *)dmaTransfer->TxData;
    }
    else
    {
        printk(KERN_ERR "[INIT][SPI] DMA buffers configuration for SPI %d failed\n", spiDeviceEnum);
    }
    Device[spiDeviceEnum].Dma.spiTransfer.cs_change = 1;
    Device[spiDeviceEnum].Dma.spiTransfer.speed_hz = Device[spiDeviceEnum].spiDevice->max_speed_hz;

    spi_message_init(&Device[spiDeviceEnum].Dma.spiMessage);
    spi_message_add_tail(&Device[spiDeviceEnum].Dma.spiTransfer, &Device[spiDeviceEnum].Dma.spiMessage);

    return 0;
}

static int spiDmaDestroy(spiDeviceType spiDeviceEnum)
{
    if (Device[spiDeviceEnum].Dma.tx_dma)
    {
        dma_unmap_single(Device[spiDeviceEnum].spiDevice->controller->dev.parent, Device[spiDeviceEnum].Dma.tx_dma, Device[spiDeviceEnum].spiLength, DMA_TO_DEVICE);
        Device[spiDeviceEnum].Dma.tx_dma = 0;
    }

    if (Device[spiDeviceEnum].Dma.rx_dma)
    {
        dma_unmap_single(Device[spiDeviceEnum].spiDevice->controller->dev.parent, Device[spiDeviceEnum].Dma.rx_dma, Device[spiDeviceEnum].spiLength, DMA_FROM_DEVICE);
        Device[spiDeviceEnum].Dma.rx_dma = 0;
    }

    return 0;
}

void transferFpgaInput(struct work_struct *work)
{
    unsigned char *tx_buf;
    unsigned char *rx_buf;
    int ret;
    int i;

    /* Initiate DMA Controller to perform SPI transfer */
    ret = spi_sync(Device[SPI_PRIMARY].spiDevice, &Device[SPI_PRIMARY].Dma.spiMessage);

    if (ret < 0)
    {
        printk(KERN_ERR "[CTRL][SPI] SPI transfer at signal From Char Device failed: %d\n", ret);
        return;
    }
    else
    {
        printk(KERN_INFO "[CTRL][SPI] Primary FPGA Transfer :: Signaled by transferFpgaInput over SPI.0\n");
    }

    /* Debug :: Dma buffer */
    tx_buf = (unsigned char *)Device[SPI_PRIMARY].Dma.spiTransfer.tx_buf;
    rx_buf = (unsigned char *)Device[SPI_PRIMARY].Dma.spiTransfer.rx_buf;
    for (i = 0; i < Device[SPI_PRIMARY].spiLength; ++i)
    {
        printk(KERN_INFO "[CTRL][SPI] Primary FPGA Transfer :: Dma Buffer[%d]: [Data] Tx[0x%02x] [Feedback] Rx[0x%02x]\n", i, tx_buf[i], rx_buf[i]);
    }

    /* No feedback preamble received :: FPGA not running properly */
    if(0x18 != rx_buf[0])
    {
        printk(KERN_ERR "[CTRL][SPI] No FPGA Preamble detected :: FPGA is Not Programed, Connected or Running properly\n");
        charDeviceLockCtrl(DEVICE_COMMANDER, CTRL_UNLOCK);
    }
}

void transferFpgaOutput(struct work_struct *work)
{
    unsigned char *tx_buf;
    unsigned char *rx_buf;
    int ret;
    int i;

    /* Initiate DMA Controller to perform SPI transfer */
    ret = spi_sync(Device[SPI_SECONDARY].spiDevice, &Device[SPI_SECONDARY].Dma.spiMessage);

    if (ret < 0)
    {
        printk(KERN_ERR "[CTRL][SPI] SPI transfer at interrupt From Fpga failed: %d\n", ret);
        return;
    }
    else
    {
        printk(KERN_INFO "[CTRL][SPI] Secondary FPGA Transfer :: Signaled by transferFpgaOutput over SPI.1\n");
    }

    /* Debug :: Dma buffer */
    tx_buf = (unsigned char *)Device[SPI_SECONDARY].Dma.spiTransfer.tx_buf;
    rx_buf = (unsigned char *)Device[SPI_SECONDARY].Dma.spiTransfer.rx_buf;
    for (i = 0; i < Device[SPI_SECONDARY].spiLength; ++i)
    {
        printk(KERN_INFO "[CTRL][SPI] Secondary FPGA Transfer :: Byte[%d]: [Feedback] Tx[0x%02x] [Data] Rx[0x%02x]\n", i, tx_buf[i], rx_buf[i]);
    }

    /* Unlock COMMANDER For Kernel Commander Device to process */
    charDeviceLockCtrl(DEVICE_COMMANDER, CTRL_UNLOCK);
}

void killApplication(struct work_struct *work)
{
    charDeviceLockCtrl(DEVICE_WATCHDOG, CTRL_UNLOCK);
}

int spiInit(void)
{
    (void)spiBusInit(BUS_SPI0, SPI_PRIMARY);
    (void)spiBusInit(BUS_SPI1, SPI_SECONDARY);

    /* Only Secondary Required :: Since primary is initialised trough State Machine */
    spiDmaInit(SPI_SECONDARY, DMA_OUT, true);

    return 0;
}

/* CONFIG */ void enableDMAConfig(void)
{
    printk(KERN_INFO "[CTRL][SPI] Conigure Primary SPI into DMA -> FPGA peripherals configuratoin\n");
    spiDmaDestroy(SPI_PRIMARY);
    spiDmaInit(SPI_PRIMARY, DMA_IN, true);
}


/* CONFIG */ void enableDMAServer(void)
{
    printk(KERN_INFO "[CTRL][SPI] Conigure Primary SPI into DMA Server Mode\n");
    spiDmaDestroy(SPI_PRIMARY);
    spiDmaInit(SPI_PRIMARY, DMA_IN, false);
}

void spiDestroy(void)
{
    spiDmaDestroy(SPI_PRIMARY);
    spiDmaDestroy(SPI_SECONDARY);

    spi_dev_put(Device[SPI_PRIMARY].spiDevice);
    spi_dev_put(Device[SPI_SECONDARY].spiDevice);
    printk(KERN_INFO "[DESTROY][SPI] Destroy SPI Devices\n");
}




