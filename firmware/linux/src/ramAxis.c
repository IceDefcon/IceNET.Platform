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
    [SECTOR_CONFIG] = 
    {
        .sectorAddress = NULL,
        .genericSize = SECTOR_AMOUNT,
    },

    [SECTOR_BMI] = 
    {
        .sectorAddress = NULL,
        .genericSize = 0,
    },

    [SECTOR_ADXL] = 
    {
        .sectorAddress = NULL,
        .genericSize = 0,
    },
};

void ramAxisInit(ramSectorType type)
{
    ramAxis[type].sectorAddress = ramDiskGetPointer(type);
    if (!ramAxis[type].sectorAddress) 
    {
        pr_err("[ERNO][RAM] Failed to get pointer to sector %d\n", type);
        return;
    }
    pr_info("[CTRL][RAM] Sector[%d] Pointer: %p\n", type, ramAxis[type].sectorAddress);
}

void ramAxisDestroy(ramSectorType type)
{
	ramDiskReleasePointer(ramAxis[type].sectorAddress);
}

void printSector(ramSectorType type)
{
    int i = 0;
    char *output;
    int offset = 0;
    int size  = 0;

    // Check if the sector address is valid
    if (!ramAxis[type].sectorAddress) 
    {
        pr_err("[ERNO][RAM] Sector %d address is NULL\n", type);
        return;
    }

    // Allocate memory for the output buffer (size 1024 bytes)
    output = kmalloc(1024, GFP_KERNEL);
    if (!output) 
    {
        pr_err("[ERNO][RAM] Failed to allocate memory for output buffer\n");
        return;
    }

    // Start the message with sector type
    offset += snprintf(output + offset, 1024 - offset, "[CTRL][RAM] Data in sector %d: ", type);

    size = ((char *)ramAxis[type].sectorAddress)[1];
    size = (size > 1024) ? 1024 : size;

    for (i = 0; i < size; ++i)
    {
        offset += snprintf(output + offset, 1024 - offset, "%02x ", ((char *)ramAxis[type].sectorAddress)[i]);
    }

    // Print the full message at once
    pr_info("%s\n", output);

    // Free the allocated memory after use
    kfree(output);
}

void* getSectorAddress(ramSectorType type)
{
    return ramAxis[type].sectorAddress;
}
