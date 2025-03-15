
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static allocationType Process;

void* memoryAllocation(uint32_t count, uint32_t size)
{
    void* ptr;

    ptr = calloc(count, size);

    Process.bytesAllocated += count * size;;
    Process.noAllocs++;

    return ptr;
}

void memoryRelease(void* ptr, uint32_t count, uint32_t size)
{
    Process.bytesAllocated -= count * size;;
    Process.noDeallocs++;
    free(ptr);
}

// int main()
// {
//     Process.noAllocs = 0;
//     Process.noDeallocs = 0;
//     Process.bytesAllocated = 0;

//     coordinatesType* ptr1 = (coordinatesType*)memoryAllocation(20, sizeof(coordinatesType));
//     coordinatesType* ptr2 = (coordinatesType*)memoryAllocation(5, sizeof(coordinatesType));
//     coordinatesType* ptr3 = (coordinatesType*)memoryAllocation(1, sizeof(coordinatesType));

//     if (ptr1 == NULL)
//     {
//         printf("Memory allocation failed\n");
//         return 1;
//     }
//     if (ptr2 == NULL)
//     {
//         printf("Memory allocation failed\n");
//         return 1;
//     }
//     if (ptr3 == NULL)
//     {
//         printf("Memory allocation failed\n");
//         return 1;
//     }

//     printf("noAllocs[%d] noDeallocs[%d] bytesAllocated[%d]\n", Process.noAllocs, Process.noDeallocs, Process.bytesAllocated);
//     memoryRelease(ptr1, 20, sizeof(coordinatesType));
//     printf("noAllocs[%d] noDeallocs[%d] bytesAllocated[%d]\n", Process.noAllocs, Process.noDeallocs, Process.bytesAllocated);
//     memoryRelease(ptr2, 5, sizeof(coordinatesType));
//     printf("noAllocs[%d] noDeallocs[%d] bytesAllocated[%d]\n", Process.noAllocs, Process.noDeallocs, Process.bytesAllocated);
//     memoryRelease(ptr3, 1, sizeof(coordinatesType));
//     printf("noAllocs[%d] noDeallocs[%d] bytesAllocated[%d]\n", Process.noAllocs, Process.noDeallocs, Process.bytesAllocated);

//     return 0;
// }



