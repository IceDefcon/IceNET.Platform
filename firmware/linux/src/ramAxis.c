/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include <linux/slab.h>
#include <linux/printk.h>

#include "ramAxis.h"
#include "ramDisk.h"

static ramAxisType ramAxis[SECTOR_AMOUNT] =
{
    [SECTOR_ENGINE] =
    {
        .sectorAddress = NULL,
        .genericSize = 0,
    },

    [SECTOR_BMI160] =
    {
        .sectorAddress = NULL,
        .genericSize = 0,
    },

    [SECTOR_ADXL345] =
    {
        .sectorAddress = NULL,
        .genericSize = 0,
    },
};

static uint8_t configBytesAmount;
static DmaTransferType* dmaTransfer;

void initTransfer(ramSectorType type)
{
    ramAxis[type].sectorAddress = ramDiskGetPointer(type);
    if (!ramAxis[type].sectorAddress) 
    {
        pr_err("[ERNO][RAM] Failed to get pointer to sector %d\n", type);
        return;
    }
}

dmaEngineType checkEngine(void)
{
    uint8_t ret = DMA_ENGINE_STOP;

    initTransfer(SECTOR_ENGINE);

    if (!ramAxis[SECTOR_ENGINE].sectorAddress)
    {
        pr_err("[ERNO][RAM] SECTOR_ENGINE address is NULL\n");
    }
    else
    {
        if(((char *)ramAxis[SECTOR_ENGINE].sectorAddress)[0] != 0x00)
        {
            /**
             *
             * TODO :: Dummy clear
             *
             * This should be copied to the local
             * structure :: ready for concatenation
             *
             */

            ret = DMA_ENGINE_READY;
        }
    }

    destroyTransfer(SECTOR_ENGINE);

    return ret;
}

void printSector(ramSectorType type)
{
    int i = 0;
    char *output;
    int offset = 0;
    int size  = 0;

    if (!ramAxis[type].sectorAddress)
    {
        pr_err("[ERNO][RAM] Sector %d address is NULL\n", type);
        return;
    }

    output = kmalloc(1024, GFP_KERNEL);
    if (!output)
    {
        pr_err("[ERNO][RAM] Failed to allocate memory for output buffer\n");
        return;
    }

    size = ((char *)ramAxis[type].sectorAddress)[0];
    size = (size > 1024) ? 1024 : size;
    size = (size < 1) ? 16 : size;

    offset += snprintf(output + offset, 1024 - offset, "[CTRL][RAM] Data in sector %d: ", type);

    for (i = 0; i < size; ++i)
    {
        offset += snprintf(output + offset, 1024 - offset, "%02x ", ((char *)ramAxis[type].sectorAddress)[i]);
    }

    pr_info("%s\n", output);
    kfree(output);
}

static void allocateTransfer(void)
{
    // Allocate memory for the dmaTransfer structure itself if it hasn't been initialized
    if (!dmaTransfer)
    {
        dmaTransfer = kmalloc(sizeof(DmaTransferType), GFP_KERNEL);
        if (!dmaTransfer)
        {
            pr_err("[ERNO][RAM] Failed to allocate memory for dmaTransfer\n");
            return;
        }
        memset(dmaTransfer, 0, sizeof(DmaTransferType));
    }

    if (!dmaTransfer->RxData)
    {
        dmaTransfer->RxData = kmalloc(BUFFER_ALLOCATION_SIZE, GFP_KERNEL);
        if (!dmaTransfer->RxData)
        {
            pr_err("[ERNO][RAM] Failed to allocate memory for dmaTransfer->RxData\n");
            return;
        }
        pr_info("[CTRL][RAM] Successfully allocated memory for dmaTransfer->RxData\n");
    }

    if (!dmaTransfer->TxData)
    {
        dmaTransfer->TxData = kmalloc(BUFFER_ALLOCATION_SIZE, GFP_KERNEL);
        if (!dmaTransfer->TxData)
        {
            pr_err("[ERNO][RAM] Failed to allocate memory for dmaTransfer->TxData\n");
            return;
        }
        pr_info("[CTRL][RAM] Successfully allocated memory for dmaTransfer->TxData\n");
    }
}

static void freeTransfer(void)
{
    if (dmaTransfer)
    {
        if (dmaTransfer->RxData)
        {
            kfree(dmaTransfer->RxData);
            dmaTransfer->RxData = NULL;
        }

        if (dmaTransfer->TxData)
        {
            kfree(dmaTransfer->TxData);
            dmaTransfer->TxData = NULL;
        }

        kfree(dmaTransfer);
        dmaTransfer = NULL;
    }
}

void ramAxisInit(void)
{
    allocateTransfer();
}

void ramAxisDestroy(void)
{
    freeTransfer();
}

static uint8_t reverseChecksum(uint8_t *data, size_t size)
{
    uint8_t i;
    uint8_t checksum = 0;
    for (i = 0; i < size; i++)
    {
        checksum ^= data[i];
#if 0 /* Checksum debug */
        pr_info("[CTRL][RAM] checksum[0x%02X] Data[0x%02X]\n", checksum, data[i]);
#endif
    }

    return checksum;
}

void prepareTransfer(ramSectorType type, bool begin, bool end)
{
    uint8_t i;
    uint8_t reversedChecksum;

    if (!ramAxis[type].sectorAddress)
    {
        pr_err("[ERNO][RAM] Sector %d address is NULL\n", type);
        return;
    }

    /**
     *
     * TODO
     *
     * Fixed 22 Bytes of data 
     * For the DMA transfer
     *
     */
    if(true == begin)
    {
        pr_info("[CTRL][RAM] Concatenate DMA Transfer\n");
        configBytesAmount = 0;
    }

    for (i = 0; i < ((char *)ramAxis[type].sectorAddress)[0]; i++)
    {
        dmaTransfer->RxData[configBytesAmount] = ((char *)ramAxis[type].sectorAddress)[i];
        configBytesAmount++;
    }

    if(true == end)
    {
        pr_info("[CTRL][RAM] Calculate DMA Transfer Reversed Checksum\n");
        reversedChecksum = reverseChecksum(dmaTransfer->RxData, configBytesAmount);

        if (0x00 == reversedChecksum)
        {
            pr_info("[CTRL][RAM] Checksum OK\n");
        }
        else
        {
            pr_err("[ERNO][RAM] Checksum ERROR :: 0x%02X \n", reversedChecksum);
        }

        pr_info("[CTRL][RAM] Assembled total of %d Bytes \n", configBytesAmount);
    }
}

void* getSectorAddress(ramSectorType type)
{
    return ramAxis[type].sectorAddress;
}

void destroyTransfer(ramSectorType type)
{
    ramDiskReleasePointer(ramAxis[type].sectorAddress);
}

/* GET */ DmaTransferType* getDmaTransfer(void)
{
    return dmaTransfer;
}

/* GET */ uint8_t getConfigBytesAmount(void)
{
    return configBytesAmount;
}
