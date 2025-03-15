#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>   // For kmalloc and kfree
#include <linux/types.h>  // For uint32_t

#include "memory.h"

static allocationType Process;

void* memoryAllocation(uint32_t count, uint32_t size)
{
    void* ptr;

    ptr = kmalloc(count * size, GFP_KERNEL);

    if (!ptr)
    {
        pr_err("Memory allocation failed\n");
        return NULL;
    }

    memset(ptr, 0, count * size);

    Process.bytesAllocated += count * size;
    Process.noAllocs++;

    return ptr;
}

void memoryRelease(void* ptr, uint32_t count, uint32_t size)
{
    if (ptr)
    {
        Process.bytesAllocated -= count * size;
        Process.noDeallocs++;

        kfree(ptr);
    }
    else
    {
        pr_err("[ERNO] [DIA] No such pointer to release\n");
    }
}

void memoryInit(void)
{
    Process.noAllocs = 0;
    Process.noDeallocs = 0;
    Process.bytesAllocated = 0;

    printk(KERN_INFO "[INIT][DIA] Allocation Monitor Initilaised\n");
}

void memoryDestroy(void)
{
    if(0 == Process.bytesAllocated)
    {
        printk(KERN_INFO "[DESTROY][DIA] Memory fully deallocated\n");

    }
    else
    {
        pr_err("[ERNO] [DIA] Memory lekage detecetd, Bytes left [%d] \n", Process.bytesAllocated);
    }
}

/* GET */ allocationType* getAllocationData(void)
{
    return &Process;
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



