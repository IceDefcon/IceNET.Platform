/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
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

void printSector(ramSectorType type, int length)
{
	int i = 0;

    if (!ramAxis[type].sectorAddress) 
    {
        pr_err("[ERNO][RAM] Sector %d address is NULL\n", type);
        return;
    }

    pr_info("[CTRL][RAM] Data in sector %d: ", type);
    for (i = 0; i < length; ++i)
    {
        pr_info("%02x ", ((char *)ramAxis[type].sectorAddress)[i]);
    }
    pr_info("\n");
}

void* getSectorAddress(ramSectorType type)
{
    return ramAxis[type].sectorAddress;
}