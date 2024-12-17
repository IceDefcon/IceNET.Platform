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
    char transfer[];
} ramAxisType;

/* INIT */ void ramAxisInit(ramSectorType type);
/* DESTROY */ void ramAxisDestroy(ramSectorType type);
/* STORE */ void processSector(ramSectorType type);
/* GET */ void* getSectorAddress(ramSectorType type);

#endif // RAM_AXIS_H
