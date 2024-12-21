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

typedef struct
{
    uint8_t size;
    uint8_t* RxData;
    uint8_t* TxData;
}dmaTransferType;

/* INIT */ void ramAxisInit(void);
/* INIT */ void initTransfer(ramSectorType type);
/* CHECK */ bool checkEngineReady(void);
/* PRINT */ void printSector(ramSectorType type);
/* PREP */ void prepareTransfer(ramSectorType type, bool begin, bool end);
/* GET */ void* getSectorAddress(ramSectorType type);
/* DESTROY */ void destroyTransfer(ramSectorType type);
/* DESTROY */ void ramAxisDestroy(void);

/* GET */  dmaTransferType* getDmaTransfer(void);

#endif // RAM_AXIS_H
