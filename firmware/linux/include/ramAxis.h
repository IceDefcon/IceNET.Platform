/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#ifndef RAM_AXIS_H
#define RAM_AXIS_H

#include <stddef.h>

typedef enum
{
    SECTOR_CONFIG,
    SECTOR_BMI,
    SECTOR_ADXL,
    SECTOR_TEST,
    SECTOR_AMOUNT,
}ramSectorType;

typedef struct
{
    void* sectorAddress;
    int genericSize;
} ramAxisType;

/* INIT */ void ramAxisInit(ramSectorType type);
/* DESTROY */ void ramAxisDestroy(ramSectorType type);
/* TEST */ void printSector(ramSectorType type, int length);
/* GET */ void* getSectorAddress(ramSectorType type);

#endif // RAM_AXIS_H
