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

void ramAxisInit(void);
void ramAxisDestroy(void);
void testPrint(void);

#endif // RAM_AXIS_H
