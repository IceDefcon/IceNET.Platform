/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#ifndef MEM_MONITOR_H
#define MEM_MONITOR_H

typedef struct
{
    uint32_t noAllocs;
    uint32_t noDeallocs;
    uint32_t bytesAllocated;
}allocationType;

void* memoryAllocation(uint32_t count, uint32_t size);
void memoryRelease(void* ptr, uint32_t count, uint32_t size);

#endif // MEM_MONITOR_H
