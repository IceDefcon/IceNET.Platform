#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>   // For kmalloc and kfree
#include <linux/types.h>  // For uint32_t

#include "memory.h"

static allocationType Process;

typedef enum
{
    MEMORY_ALLOCATION,
    MEMORY_DEALLOCATION,
    MEMORY_AMOUNT,
} memoryAllocationType;

/* PRINT */ static void modAllocation(memoryAllocationType type)
{
    if(MEMORY_ALLOCATION == type)
    {
        printk(KERN_INFO "[INFO][DIA] Allocation [%d => %d] Bytes\n", Process.prevAllocated, Process.currAllocated);
    }
    else if(MEMORY_DEALLOCATION == type)
    {
        printk(KERN_INFO "[INFO][DIA] Deallocation [%d => %d] Bytes\n", Process.prevAllocated, Process.currAllocated);
    }
}

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

    Process.prevAllocated = Process.currAllocated;
    Process.currAllocated += count * size;
    Process.noAllocs++;

    modAllocation(MEMORY_ALLOCATION);
    return ptr;
}

void memoryRelease(void* ptr, uint32_t count, uint32_t size)
{
    if (ptr)
    {
        Process.prevAllocated = Process.currAllocated;
        Process.currAllocated -= count * size;
        Process.noDeallocs++;

        kfree(ptr);
    }
    else
    {
        pr_err("[ERNO] [DIA] No such pointer to release\n");
    }
    modAllocation(MEMORY_DEALLOCATION);
}

void memoryInit(void)
{
    Process.noAllocs = 0;
    Process.noDeallocs = 0;
    Process.currAllocated = 0;

    printk(KERN_INFO "[INIT][DIA] Allocation Monitor Initilaised\n");
}

void memoryDestroy(void)
{
    if(0 == Process.currAllocated)
    {
        printk(KERN_INFO "[DESTROY][DIA] Memory Deallocated Successfully\n");
    }
    else
    {
        pr_err("[ERNO] [DIA] Memory Lekage Detected\n");
        showAllocation();
    }
}

/* PRINT */ void showAllocation(void)
{
    printk(KERN_INFO "[INFO][DIA] Allocations[%d] Deallocatios[%d] Allocated[%d] Bytes\n", Process.noAllocs, Process.noDeallocs, Process.currAllocated);
}
