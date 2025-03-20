/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */

#ifndef RAM_AXIS_H
#define RAM_AXIS_H

#include <stddef.h>

#include "types.h"

#define DMA_ENGINE_SIZE 4 /* Size of DMA Engine control bytes */
#define RAM_SECTOR_SIZE 512 /* Number of bytes per sector */

#define DMA_BUFFER_ALLOCATION_SIZE 128

typedef enum
{
    SECTOR_ENGINE,
    SECTOR_BMI160_0,
    SECTOR_BMI160_1,
    SECTOR_ADXL345,
    SECTOR_AMOUNT,
}ramSectorType;

typedef enum
{
    DMA_ENGINE_STOP,
    DMA_ENGINE_READY,
    DMA_ENGINE_AMOUNT
}dmaEngineType;

typedef struct
{
    void* sectorAddress;
    int genericSize;
} sectorType;

typedef struct
{
    uint8_t configBytesAmount;
    DmaTransferType* dmaTransfer;
    sectorType sector[SECTOR_AMOUNT];
} ramAxisType;

/* INIT */ void ramAxisInit(void);
/* INIT */ void initTransfer(ramSectorType type);
/* CHECK */ dmaEngineType checkEngine(void);
/* PRINT */ void printSector(ramSectorType type);
/* PREP */ void prepareTransfer(ramSectorType type, bool begin, bool end);
/* GET */ void* getSectorAddress(ramSectorType type);
/* DESTROY */ void destroyTransfer(ramSectorType type);
/* DESTROY */ void ramAxisDestroy(void);

/* GET */ DmaTransferType* getDmaTransfer(void);
/* GET */ uint8_t getConfigBytesAmount(void);

#endif // RAM_AXIS_H
