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
        [SECTOR_BMI160_0] = { .sectorAddress = NULL },
        [SECTOR_BMI160_1] = { .sectorAddress = NULL },
        [SECTOR_ADXL345] = { .sectorAddress = NULL },
    },

    .payloadBytes = 0,
    .payloadReady = false
};

static void initTransfer(ramSectorType type)
{
    ramAxis.sector[type].sectorAddress = ramDiskGetPointer(type);
    if (!ramAxis.sector[type].sectorAddress)
    {
        pr_err("[ERNO][RAM] Failed to get pointer to sector %d\n", type);
        return;
    }
}

static uint8_t reverseChecksum(uint8_t *data, size_t size)
{
    uint8_t i;
    uint8_t checksum = 0;
    for (i = 0; i < size; i++)
    {
        checksum ^= data[i];
#if 0 /* Debug */
        pr_info("[CTRL][RAM] checksum[0x%02X] Data[0x%02X]\n", checksum, data[i]);
#endif
    }

    return checksum;
}

static void prepareTransfer(ramSectorType type, bool begin, bool end)
{
    uint8_t i;
    uint8_t reversedChecksum;

    if (!ramAxis.sector[type].sectorAddress)
    {
        pr_err("[ERNO][RAM] Sector %d address is NULL\n", type);
        return;
    }

    if(true == begin)
    {
        pr_info("[CTRL][RAM] Concatenate DMA Transfer\n");
        ramAxis.configBytesAmount = 0;
        ramAxis.payloadBytes = 0;
        ramAxis.payloadReady = false;
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

        if(ramAxis.configBytesAmount > DMA_BUFFER_ALLOCATION_SIZE)
        {
            pr_info("[CTRL][RAM] Assembled DMA Data [%d] Bytes \n", ramAxis.configBytesAmount);
            pr_err("[ERNO][RAM] Assembled DMA bigger than allocated buffer [%d] Bytes\n", DMA_BUFFER_ALLOCATION_SIZE);
        }
        else
        {
            pr_info("[CTRL][RAM] Assembled DMA Data [%d] Bytes \n", ramAxis.configBytesAmount);
        }

        /**
         * Together with payloadbytes used to verify
         * that State Machines of I2C and SPI controllers
         * send back the feedback data indicating process complete
         *
         *
         */
        ramAxis.payloadBytes += ramAxis.configBytesAmount - 4 - 5*(SECTOR_AMOUNT - 1);
        ramAxis.payloadReady = true;
    }
}

static void destroyTransfer(ramSectorType type)
{
    ramDiskReleasePointer(ramAxis.sector[type].sectorAddress);
}

static void printSector(ramSectorType type)
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
        ramAxis.dmaTransfer->RxData = (uint8_t*)memoryAllocation(DMA_BUFFER_ALLOCATION_SIZE, sizeof(uint8_t));
        if (!ramAxis.dmaTransfer->RxData)
        {
            pr_err("[ERNO][RAM] Failed to allocate memory for ramAxis.dmaTransfer->RxData buffer\n");
            return;
        }
        pr_info("[INIT][RAM] Successfully allocated memory for ramAxis.dmaTransfer->RxData buffer\n");
    }

    if (!ramAxis.dmaTransfer->TxData)
    {
        ramAxis.dmaTransfer->TxData = (uint8_t*)memoryAllocation(DMA_BUFFER_ALLOCATION_SIZE, sizeof(uint8_t));
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
            memoryRelease(ramAxis.dmaTransfer->RxData, DMA_BUFFER_ALLOCATION_SIZE, sizeof(char));
            ramAxis.dmaTransfer->RxData = NULL;
        }

        if (ramAxis.dmaTransfer->TxData)
        {
            memoryRelease(ramAxis.dmaTransfer->TxData, DMA_BUFFER_ALLOCATION_SIZE, sizeof(char));
            ramAxis.dmaTransfer->TxData = NULL;
        }

        memoryRelease(ramAxis.dmaTransfer, 1, sizeof(DmaTransferType));
        ramAxis.dmaTransfer = NULL;
    }
}

/* INIT */ void ramAxisInit(void)
{
    allocateTransfer();
}

/* DESTROY */ void ramAxisDestroy(void)
{
    freeTransfer();
}

/* PREP */ void prepareRamDiskTransfer(void)
{
    /* Init pointers */
    initTransfer(SECTOR_ENGINE);
    initTransfer(SECTOR_BMI160_0);
    initTransfer(SECTOR_BMI160_1);
    initTransfer(SECTOR_ADXL345);
    /* Prepare DMA Transfer */
    prepareTransfer(SECTOR_ENGINE, true, false);
    prepareTransfer(SECTOR_BMI160_0, false, false);
    prepareTransfer(SECTOR_BMI160_1, false, false);
    prepareTransfer(SECTOR_ADXL345, false, true);
    /* Destroy life-time pointers */
    destroyTransfer(SECTOR_ENGINE);
    destroyTransfer(SECTOR_BMI160_0);
    destroyTransfer(SECTOR_BMI160_1);
    destroyTransfer(SECTOR_ADXL345);
}

/* PRINT */ void printRamDiskData(void)
{
    /*
     * [0] :: DMA Engine Config
     * [1] :: DMA BMI160 Config
     * [2] :: DMA BMI160 Config
     */
    initTransfer(SECTOR_ENGINE);
    initTransfer(SECTOR_BMI160_0);
    initTransfer(SECTOR_BMI160_1);
    initTransfer(SECTOR_ADXL345);
    printSector(SECTOR_ENGINE);
    printSector(SECTOR_BMI160_0);
    printSector(SECTOR_BMI160_1);
    printSector(SECTOR_ADXL345);
    destroyTransfer(SECTOR_ENGINE);
    destroyTransfer(SECTOR_BMI160_0);
    destroyTransfer(SECTOR_BMI160_1);
    destroyTransfer(SECTOR_ADXL345);
}

/* GET */ void* getSectorAddress(ramSectorType type)
{
    return ramAxis.sector[type].sectorAddress;
}

/* GET */ DmaTransferType* getDmaTransfer(void)
{
    return ramAxis.dmaTransfer;
}

/* GET */ uint8_t getConfigBytesAmount(void)
{
    return ramAxis.configBytesAmount;
}

/* GET */ uint8_t getPayloadBytesAmount(void)
{
    return ramAxis.payloadBytes;
}

/* IS */ bool isPayloadReady(void)
{
    return ramAxis.payloadReady;
}
