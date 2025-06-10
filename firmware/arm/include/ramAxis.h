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
    uint8_t payloadBytes;
    bool payloadReady;
} ramAxisType;

/* INIT */ void ramAxisInit(void);
/* DESTROY */ void ramAxisDestroy(void);

/* PREP */ void prepareRamDiskTransfer(void);
/* PRINT */ void printRamDiskData(void);

/* GET */ void* getSectorAddress(ramSectorType type);
/* GET */ DmaTransferType* getRamdiskDmaTransfer(void);
/* GET */ uint8_t getRamdiskConfigTransferSize(void);
/* GET */ uint8_t getPayloadBytesAmount(void);
/* IS */ bool isPayloadReady(void);

#endif // RAM_AXIS_H
