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

void ramAxisInit(void)
{
    int i;

    for (i = 0; i < SECTOR_AMOUNT; ++i) 
    {
        ramAxis[i].sectorAddress = ramDiskGetPointer(i);
        if (!ramAxis[i].sectorAddress) 
        {
            pr_err("[ERNO][RAM] Failed to get pointer to sector %d\n", i);
            return;
        }
        pr_info("[CTRL][RAM] Sector[%d] Pointer: %p\n", i, ramAxis[i].sectorAddress);
    }
}

void ramAxisDestroy(void)
{
	ramDiskReleasePointer(ramAxis[SECTOR_CONFIG].sectorAddress);
	ramDiskReleasePointer(ramAxis[SECTOR_BMI].sectorAddress);
	ramDiskReleasePointer(ramAxis[SECTOR_ADXL].sectorAddress);
	ramDiskReleasePointer(ramAxis[SECTOR_TEST].sectorAddress);
}

void testPrint(void)
{
	int i = 0;

    if (!ramAxis[SECTOR_TEST].sectorAddress) 
    {
        pr_err("[ERNO][RAM] Sector TEST address is NULL\n");
        return;
    }

    for (i = 0; i < 4; ++i)
    {
	    pr_info("[CTRL][RAM] Data in sector 3: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", 
	            ((char *)ramAxis[i].sectorAddress)[0], 
	            ((char *)ramAxis[i].sectorAddress)[1], 
	            ((char *)ramAxis[i].sectorAddress)[2], 
	            ((char *)ramAxis[i].sectorAddress)[3], 
	            ((char *)ramAxis[i].sectorAddress)[4], 
	            ((char *)ramAxis[i].sectorAddress)[5], 
	            ((char *)ramAxis[i].sectorAddress)[6], 
	            ((char *)ramAxis[i].sectorAddress)[7], 
	            ((char *)ramAxis[i].sectorAddress)[8], 
	            ((char *)ramAxis[i].sectorAddress)[9], 
	            ((char *)ramAxis[i].sectorAddress)[10], 
	            ((char *)ramAxis[i].sectorAddress)[11], 
	            ((char *)ramAxis[i].sectorAddress)[12], 
	            ((char *)ramAxis[i].sectorAddress)[13], 
	            ((char *)ramAxis[i].sectorAddress)[14], 
	            ((char *)ramAxis[i].sectorAddress)[15]);
    }

}