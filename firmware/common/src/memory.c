#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>   // For kmalloc and kfree
#include <linux/types.h>  // For uint32_t
#include <linux/kallsyms.h>
#include <linux/sched.h>
#include <asm/thread_info.h>  // For THREAD_SIZE
#include <linux/version.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#define STACK_PATTERN 0xAA  // Common poison pattern used to initialize stacks

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
// Include for 5.4.0 and newer kernels (like 5.4.0-150-generic)
#include <linux/sched/task_stack.h>
#endif

#include "memory.h"

static allocationType Process =
{
    .currAllocated = 0,
    .prevAllocated = 0,
    .noAllocs = 0,
    .noDeallocs = 0,
};

/* PRINT */ static void modAllocation(memoryAllocationType type)
{
    if(MEMORY_ALLOCATION == type)
    {
        printk(KERN_INFO "[INIT][DIA] Allocation [%d => %d] Bytes\n", Process.prevAllocated, Process.currAllocated);
    }
    else if(MEMORY_DEALLOCATION == type)
    {
        printk(KERN_INFO "[DESTROY][DIA] Deallocation [%d => %d] Bytes\n", Process.prevAllocated, Process.currAllocated);
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
    Process.currAllocated = 0;
    Process.prevAllocated = 0;
    Process.noAllocs = 0;
    Process.noDeallocs = 0;

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// Thread Stack Layout (e.g., 16 KB typical):
//
// Higher address
// ┌─────────────────────────────┐
// │         Stack End           │ ← task_stack_page() + THREAD_SIZE
// │                             │
// │   Stack frames grow down    │ ← current stack pointer (sp)
// │                             │
// │         Stack Base          │ ← task_stack_page(current)
// └─────────────────────────────┘
// Lower address
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* PRINT */ void showThreadDiagnostics(const char name[])
{
    size_t estimated_max_usage = 0;
    unsigned long sp = (unsigned long)&sp; /* Local variable address :: Current stack pointer */
    unsigned long stack_base = (unsigned long)task_stack_page(current); /* Base address of the current task’s stack */
    unsigned long stack_end = stack_base + THREAD_SIZE; /* Top of the stack */

    size_t used = stack_end - sp;
    size_t remaining = sp - stack_base;

    // Estimate high water mark
    unsigned char *stack_ptr = (unsigned char *)stack_base;
    size_t high_water_bytes = 0;
    while ((unsigned long)stack_ptr < stack_end && *stack_ptr == STACK_PATTERN)
    {
        high_water_bytes++;
        stack_ptr++;
    }

    estimated_max_usage = THREAD_SIZE - high_water_bytes;

    pr_info("[INFO][DIA] Kernel Thread ID   : %s\n", name);
    pr_info("[INFO][DIA] Kernel Stack Base  : 0x%lx\n", stack_base);
    pr_info("[INFO][DIA] Kernel Stack End   : 0x%lx\n", stack_end);
    pr_info("[INFO][DIA] Current Stack Ptr  : 0x%lx THREAD_SIZE[0x%lx]\n", sp, (unsigned long)THREAD_SIZE);
    pr_info("[INFO][DIA] Used Stack         : %zu bytes\n", used);
    pr_info("[INFO][DIA] Free Stack         : %zu bytes\n", remaining);
    pr_info("[INFO][DIA] High Water Mark    : %zu bytes used (max so far)\n", estimated_max_usage);
}

/* PRINT */ void showModuleDiagnostics(const char name[])
{
    unsigned long sp = (unsigned long)&sp;  // Current stack pointer (local variable address)
    unsigned long stack_base = (unsigned long)task_stack_page(current);  // Stack base of current task
    unsigned long stack_end = stack_base + THREAD_SIZE;  // Stack top

    size_t used = stack_end - sp;
    size_t remaining = sp - stack_base;

    pr_info("[INFO][MOD] Kernel Module Context: %s\n", name);
    pr_info("[INFO][MOD] Stack Base  : 0x%lx\n", stack_base);
    pr_info("[INFO][MOD] Stack End   : 0x%lx\n", stack_end);
    pr_info("[INFO][MOD] Stack Ptr   : 0x%lx\n", sp);
    pr_info("[INFO][MOD] Used Stack  : %zu bytes\n", used);
    pr_info("[INFO][MOD] Free Stack  : %zu bytes\n", remaining);
}

/* PRINT */ void showSections(void)
{
    void *text_start = (void *)kallsyms_lookup_name("_stext");
    void *data_start = (void *)kallsyms_lookup_name("_sdata");
    void *bss_start  = (void *)kallsyms_lookup_name("__bss_start");

    void *text_end   = (void *)kallsyms_lookup_name("_etext");
    void *data_end   = (void *)kallsyms_lookup_name("_edata");
    void *bss_end    = (void *)kallsyms_lookup_name("__bss_stop");

    size_t text_size = (size_t)(text_end - text_start);
    size_t data_size = (size_t)(data_end - data_start);
    size_t bss_size  = (size_t)(bss_end  - bss_start);

    pr_info("[INFO][DIA] .text %p - %p [Size: %zu bytes]\n", text_start, text_end, text_size);
    pr_info("[INFO][DIA] .data %p - %p [Size: %zu bytes]\n", data_start, data_end, data_size);
    pr_info("[INFO][DIA] .bss  %p - %p [Size: %zu bytes]\n", bss_start, bss_end, bss_size);
}

/* PRINT */ void showAllocation(void)
{
    printk(KERN_INFO "[INFO][DIA] Allocations[%d] Deallocatios[%d] Allocated[%d] Bytes\n", Process.noAllocs, Process.noDeallocs, Process.currAllocated);
}
