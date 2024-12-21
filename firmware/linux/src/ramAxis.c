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

typedef struct
{
    /**
     *
     *
     *
     *
     * TODO
     *
     *
     *
     *
     *
     */
} ramAxisConfig;

static ramAxisConfig AxisConfig =
{

};

void ramAxisInit(ramSectorType type)
{
    ramAxis[type].sectorAddress = ramDiskGetPointer(type);
    if (!ramAxis[type].sectorAddress) 
    {
        pr_err("[ERNO][RAM] Failed to get pointer to sector %d\n", type);
        return;
    }
    // pr_info("[CTRL][RAM] Sector[%d] Pointer: %p\n", type, ramAxis[type].sectorAddress);
}

bool checkEngineReady(void)
{
    bool ret = false;

    ramAxisInit(SECTOR_ENGINE);

    if (!ramAxis[SECTOR_ENGINE].sectorAddress)
    {
        pr_err("[ERNO][RAM] SECTOR_ENGINE address is NULL\n");
    }
    else
    {
        if(((char *)ramAxis[SECTOR_ENGINE].sectorAddress)[0] != 0x00)
        {
            pr_info("[CTRL][RAM] DMA Ready to Launch\n");

            /**
             *
             * TODO :: Dummy clear
             *
             * This should be copied to the local
             * structure :: ready for concatenation
             *
             */

            ret = true;
        }
    }

    ramAxisDestroy(SECTOR_ENGINE);

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

static uint8_t j;
static uint8_t * dmaData;

uint8_t * getDmaData(void)
{
    return dmaData;
}

void allocateTransfer(void)
{
    dmaData = kmalloc(22 + 1, GFP_KERNEL);
}

static uint8_t reverseChecksum(uint8_t *data, size_t size)
{
    uint8_t i;
    uint8_t checksum = 0;

    for (i = 0; i < size; i++)
    {
        checksum ^= data[i];
        pr_info("[CTRL][RAM] checksum[0x%02X] Data[0x%02X]\n", checksum, data[i]);
    }

    return checksum;
}

void prepareTransfer(ramSectorType type, bool begin, bool end)
{
    uint8_t i;

    if (!ramAxis[type].sectorAddress)
    {
        pr_err("[ERNO][RAM] Sector %d address is NULL\n", type);
        return;
    }

    /**
     *
     * TODO
     *
     * Fixed 22 + 1 bytes
     * As we know this the current
     * size of the DMA transfer
     * and 1 byte for chacksum
     *
     */
    if(true == begin)
    {
        pr_info("[CTRL][RAM] Concatenate DMA Transfer\n");
        allocateTransfer();
        j = 0;
    }

    for (i = 0; i < ((char *)ramAxis[type].sectorAddress)[0]; i++)
    {
        dmaData[j] = ((char *)ramAxis[type].sectorAddress)[i];
        j++;
    }

    if(true == end)
    {
        pr_info("[CTRL][RAM] Calculating Dma transfer checksum\n");
        dmaData[j] = reverseChecksum(dmaData, j);
        j = 0;
    }
}

void* getSectorAddress(ramSectorType type)
{
    return ramAxis[type].sectorAddress;
}

void ramAxisDestroy(ramSectorType type)
{
    ramDiskReleasePointer(ramAxis[type].sectorAddress);
}
