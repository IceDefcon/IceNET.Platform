/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#ifndef RAM_AXIS_H
#define RAM_AXIS_H

#include <stddef.h>

#define DMA_ENGINE_SIZE 4 /* Size of DMA Engine control bytes */
#define RAM_SECTOR_SIZE 512 /* Number of bytes per sector */

typedef enum
{
    SECTOR_ENGINE,
    SECTOR_BMI160,
    SECTOR_ADXL345,
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
/* ENGINE */ void processEngine(ramSectorType type);
/* STORE */ void processSector(ramSectorType type);
/* GET */ void* getSectorAddress(ramSectorType type);

#endif // RAM_AXIS_H
