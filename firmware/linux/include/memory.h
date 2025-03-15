/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#ifndef MEMORY_H
#define MEMORY_H

typedef struct
{
    uint32_t bytesAllocated;
    uint32_t noAllocs;
    uint32_t noDeallocs;
} allocationType;

void memoryInit(void);
void memoryDestroy(void);

void* memoryAllocation(uint32_t count, uint32_t size);
void memoryRelease(void* ptr, uint32_t count, uint32_t size);

/* GET */ allocationType* getAllocationData(void);

#endif // MEMORY_H
