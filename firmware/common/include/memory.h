/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#ifndef MEMORY_H
#define MEMORY_H

typedef enum
{
    MEMORY_ALLOCATION,
    MEMORY_DEALLOCATION,
    MEMORY_AMOUNT,
} memoryAllocationType;

typedef struct
{
    uint32_t currAllocated;
    uint32_t prevAllocated;
    uint32_t noAllocs;
    uint32_t noDeallocs;
} allocationType;

void memoryInit(void);
void memoryDestroy(void);

void* memoryAllocation(uint32_t count, uint32_t size);
void memoryRelease(void* ptr, uint32_t count, uint32_t size);

/* PRINT */ void showThreadDiagnostics(const char name[]);
/* PRINT */ void showModuleDiagnostics(const char name[]);
/* PRINT */ void showSections(void);
/* PRINT */ void showAllocation(void);

#endif // MEMORY_H
