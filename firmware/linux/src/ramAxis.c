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
#include "memory.h"

static ramAxisType ramAxis =
{
    .configBytesAmount = 0,
    .dmaTransfer = NULL,

    .sector =
    {
        [SECTOR_ENGINE] = { .sectorAddress = NULL },
        [SECTOR_BMI160] = { .sectorAddress = NULL },
        [SECTOR_ADXL345] = { .sectorAddress = NULL },
    }
};

void initTransfer(ramSectorType type)
{
    ramAxis.sector[type].sectorAddress = ramDiskGetPointer(type);
    if (!ramAxis.sector[type].sectorAddress)
    {
        pr_err("[ERNO][RAM] Failed to get pointer to sector %d\n", type);
        return;
    }
}

dmaEngineType checkEngine(void)
{
    uint8_t ret = DMA_ENGINE_STOP;

    initTransfer(SECTOR_ENGINE);

    if (!ramAxis.sector[SECTOR_ENGINE].sectorAddress)
    {
        pr_err("[ERNO][RAM] SECTOR_ENGINE address is NULL\n");
    }
    else
    {
        if(((char *)ramAxis.sector[SECTOR_ENGINE].sectorAddress)[0] != 0x00)
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

    if (!ramAxis.sector[type].sectorAddress)
    {
        pr_err("[ERNO][RAM] Sector %d address is NULL\n", type);
        return;
    }

    output = (char*)memoryAllocation(1024, sizeof(char));
    if (!output)
    {
        pr_err("[ERNO][RAM] Failed to allocate memory for output buffer\n");
        return;
    }

    size = ((char *)ramAxis.sector[type].sectorAddress)[0];
    size = (size > 1024) ? 1024 : size;
    size = (size < 1) ? 16 : size;

    offset += snprintf(output + offset, 1024 - offset, "[CTRL][RAM] Data in sector %d: ", type);

    for (i = 0; i < size; ++i)
    {
        offset += snprintf(output + offset, 1024 - offset, "%02x ", ((char *)ramAxis.sector[type].sectorAddress)[i]);
    }

    pr_info("%s\n", output);
    memoryRelease(output, 1024, sizeof(char));
}

static void allocateTransfer(void)
{
    // Allocate memory for the dmaTransfer structure itself if it hasn't been initialized
    if (!ramAxis.dmaTransfer)
    {
        ramAxis.dmaTransfer = (DmaTransferType*)memoryAllocation(1, sizeof(DmaTransferType));
        if (!ramAxis.dmaTransfer)
        {
            pr_err("[ERNO][RAM] Failed to allocate memory for ramAxis.dmaTransfer\n");
            return;
        }
        pr_info("[INIT][RAM] Successfully allocated memory for DmaTransferType\n");
        memset(ramAxis.dmaTransfer, 0, sizeof(DmaTransferType));
    }

    if (!ramAxis.dmaTransfer->RxData)
    {
        ramAxis.dmaTransfer->RxData = (uint8_t*)memoryAllocation(BUFFER_ALLOCATION_SIZE, sizeof(uint8_t));
        if (!ramAxis.dmaTransfer->RxData)
        {
            pr_err("[ERNO][RAM] Failed to allocate memory for ramAxis.dmaTransfer->RxData buffer\n");
            return;
        }
        pr_info("[INIT][RAM] Successfully allocated memory for ramAxis.dmaTransfer->RxData buffer\n");
    }

    if (!ramAxis.dmaTransfer->TxData)
    {
        ramAxis.dmaTransfer->TxData = (uint8_t*)memoryAllocation(BUFFER_ALLOCATION_SIZE, sizeof(uint8_t));
        if (!ramAxis.dmaTransfer->TxData)
        {
            pr_err("[ERNO][RAM] Failed to allocate memory for ramAxis.dmaTransfer->TxData buffer\n");
            return;
        }
        pr_info("[INIT][RAM] Successfully allocated memory for ramAxis.dmaTransfer->TxData buffer\n");
    }
}

static void freeTransfer(void)
{
    if (ramAxis.dmaTransfer)
    {
        if (ramAxis.dmaTransfer->RxData)
        {
            memoryRelease(ramAxis.dmaTransfer->RxData, BUFFER_ALLOCATION_SIZE, sizeof(char));
            ramAxis.dmaTransfer->RxData = NULL;
        }

        if (ramAxis.dmaTransfer->TxData)
        {
            memoryRelease(ramAxis.dmaTransfer->TxData, BUFFER_ALLOCATION_SIZE, sizeof(char));
            ramAxis.dmaTransfer->TxData = NULL;
        }

        memoryRelease(ramAxis.dmaTransfer, 1, sizeof(DmaTransferType));
        ramAxis.dmaTransfer = NULL;
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

    if (!ramAxis.sector[type].sectorAddress)
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
        ramAxis.configBytesAmount = 0;
    }

    for (i = 0; i < ((char *)ramAxis.sector[type].sectorAddress)[0]; i++)
    {
        ramAxis.dmaTransfer->RxData[ramAxis.configBytesAmount] = ((char *)ramAxis.sector[type].sectorAddress)[i];
        ramAxis.configBytesAmount++;
    }

    if(true == end)
    {
        pr_info("[CTRL][RAM] Calculate DMA Transfer Reversed Checksum\n");
        reversedChecksum = reverseChecksum(ramAxis.dmaTransfer->RxData, ramAxis.configBytesAmount);

        if (0x00 == reversedChecksum)
        {
            pr_info("[CTRL][RAM] Checksum OK\n");
        }
        else
        {
            pr_err("[ERNO][RAM] Checksum ERROR :: 0x%02X \n", reversedChecksum);
        }

        pr_info("[CTRL][RAM] Assembled total of %d Bytes \n", ramAxis.configBytesAmount);
    }
}

void* getSectorAddress(ramSectorType type)
{
    return ramAxis.sector[type].sectorAddress;
}

void destroyTransfer(ramSectorType type)
{
    ramDiskReleasePointer(ramAxis.sector[type].sectorAddress);
}

/* GET */ DmaTransferType* getDmaTransfer(void)
{
    return ramAxis.dmaTransfer;
}

/* GET */ uint8_t getConfigBytesAmount(void)
{
    return ramAxis.configBytesAmount;
}
