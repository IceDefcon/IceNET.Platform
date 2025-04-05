/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */

#include "spiDmaCtrl.h"
#include "scheduler.h"
#include "spiWork.h"
#include "memory.h"

//////////////////////
//                  //
//                  //
//                  //
//   [W] Workload   //
//                  //
//                  //
//                  //
//////////////////////

static workTaskData workTask[WORK_AMOUNT] =
{
	[WORK_MASTER_PRIMARY] =
	{
		.workQueue = NULL,
		.workUnit = NULL,
	},

	[WORK_MASTER_SECONDARY] =
	{
		.workQueue = NULL,
		.workUnit = NULL,
	},

	[WORK_SCHEDULE_TIMER] =
	{
		.workQueue = NULL,
		.workUnit = NULL,
	},
};

/* GET WORK QUEUE*/ struct workqueue_struct* get_masterTransferPrimary_wq(void){return workTask[WORK_MASTER_PRIMARY].workQueue;}
/* GET WORK QUEUE */ struct workqueue_struct* get_masterTransferSecondary_wq(void){return workTask[WORK_MASTER_SECONDARY].workQueue;}
/* GET WORK QUEUE */ struct workqueue_struct* get_scheduleTimer_wq(void){return workTask[WORK_SCHEDULE_TIMER].workQueue;}

/* GET WORK */ struct work_struct* get_masterTransferPrimary_work(void){return workTask[WORK_MASTER_PRIMARY].workUnit;}
/* GET WORK */ struct work_struct* get_masterTransferSecondary_work(void){return workTask[WORK_MASTER_SECONDARY].workUnit;}
/* GET WORK */ struct work_struct* get_scheduleTimer_work(void){return workTask[WORK_SCHEDULE_TIMER].workUnit;}

static void masterTransferPrimary_WorkInit(void)
{

    printk(KERN_ERR "[INIT][WRK] masterTransferPrimary :: Init work unit\n");
	workTask[WORK_MASTER_PRIMARY].workUnit = (struct work_struct*)memoryAllocation(1, sizeof(struct work_struct));
    if (!workTask[WORK_MASTER_PRIMARY].workUnit)
    {
        printk(KERN_ERR "[INIT][WRK] Failed to allocate memory for masterTransferPrimary work unit: -ENOMEM\n");
		memoryRelease(workTask[WORK_MASTER_PRIMARY].workUnit, 1, sizeof(struct work_struct));
    }
	else
	{
		printk(KERN_ERR "[INIT][WRK] masterTransferPrimary :: Memory allocattion successfully\n");
	}

	INIT_WORK(workTask[WORK_MASTER_PRIMARY].workUnit, masterTransferPrimary);
	workTask[WORK_MASTER_PRIMARY].workQueue = create_singlethread_workqueue("masterTransferPrimary_workqueue");
	if (!workTask[WORK_MASTER_PRIMARY].workQueue)
	{
	    printk(KERN_ERR "[INIT][WRK] Failed to initialise single thread workqueue for masterTransferPrimary: -ENOMEM\n");
	}
	else
	{
		printk(KERN_ERR "[INIT][WRK] masterTransferPrimary :: Single thread workqueue created successfully\n");
	}
}

static void masterTransferSecondary_WorkInit(void)
{
    printk(KERN_ERR "[INIT][WRK] masterTransferSecondary :: Init work unit\n");
	workTask[WORK_MASTER_SECONDARY].workUnit = (struct work_struct*)memoryAllocation(1, sizeof(struct work_struct));
    if (!workTask[WORK_MASTER_SECONDARY].workUnit)
    {
        printk(KERN_ERR "[INIT][WRK] Failed to allocate memory for masterTransferSecondary work unit: -ENOMEM\n");
		memoryRelease(workTask[WORK_MASTER_SECONDARY].workUnit, 1, sizeof(struct work_struct));
    }
	else
	{
		printk(KERN_ERR "[INIT][WRK] masterTransferSecondary :: Memory allocattion successfully\n");
	}

	INIT_WORK(workTask[WORK_MASTER_SECONDARY].workUnit, masterTransferSecondary);
	workTask[WORK_MASTER_SECONDARY].workQueue = create_singlethread_workqueue("masterTransferPrimary_workqueue");
	if (!workTask[WORK_MASTER_SECONDARY].workQueue)
	{
	    printk(KERN_ERR "[INIT][WRK] Failed to initialise single thread workqueue for masterTransferSecondary: -ENOMEM\n");
	}
	else
	{
		printk(KERN_ERR "[INIT][WRK] masterTransferSecondary :: Single thread workqueue created successfully\n");
	}
}

static void scheduleTimer_WorkInit(void)
{
    printk(KERN_ERR "[INIT][WRK] scheduleTimer :: Init work unit\n");
    workTask[WORK_SCHEDULE_TIMER].workUnit = (struct work_struct*)memoryAllocation(1, sizeof(struct work_struct));
    if (!workTask[WORK_SCHEDULE_TIMER].workUnit)
    {
        printk(KERN_ERR "[INIT][WRK] Failed to allocate memory for scheduleTimer work unit: -ENOMEM\n");
        memoryRelease(workTask[WORK_SCHEDULE_TIMER].workUnit, 1, sizeof(struct work_struct));
    }
    else
    {
        printk(KERN_ERR "[INIT][WRK] scheduleTimer :: Memory allocattion successfully\n");
    }

    INIT_WORK(workTask[WORK_SCHEDULE_TIMER].workUnit, schedulerTimerRun);
    workTask[WORK_SCHEDULE_TIMER].workQueue = create_singlethread_workqueue("scheduleTimer_workqueue");
    if (!workTask[WORK_SCHEDULE_TIMER].workQueue)
    {
        printk(KERN_ERR "[INIT][WRK] Failed to initialise single thread workqueue for scheduleTimer: -ENOMEM\n");
    }
    else
    {
        printk(KERN_ERR "[INIT][WRK] scheduleTimer :: Single thread workqueue created successfully\n");
    }
}

static void masterTransferPrimary_WorkDestroy(void)
{
    cancel_work_sync(workTask[WORK_MASTER_PRIMARY].workUnit);
    if (workTask[WORK_MASTER_PRIMARY].workQueue)
    {
        flush_workqueue(workTask[WORK_MASTER_PRIMARY].workQueue);
        destroy_workqueue(workTask[WORK_MASTER_PRIMARY].workQueue);
        workTask[WORK_MASTER_PRIMARY].workQueue = NULL;
    }

    if (workTask[WORK_MASTER_PRIMARY].workUnit)
    {
        printk(KERN_ERR "[DESTROY][WRK] Primary workUnit Deallocated\n");
		memoryRelease(workTask[WORK_MASTER_PRIMARY].workUnit, 1, sizeof(struct work_struct));
    }

    printk(KERN_ERR "[DESTROY][WRK] Work unit :: masterTransferPrimary\n");
}

static void masterTransferSecondary_WorkDestroy(void)
{
    cancel_work_sync(workTask[WORK_MASTER_SECONDARY].workUnit);
    if (workTask[WORK_MASTER_SECONDARY].workQueue)
    {
        flush_workqueue(workTask[WORK_MASTER_SECONDARY].workQueue);
        destroy_workqueue(workTask[WORK_MASTER_SECONDARY].workQueue);
        workTask[WORK_MASTER_SECONDARY].workQueue = NULL;
    }

    if (workTask[WORK_MASTER_SECONDARY].workUnit)
    {
        printk(KERN_ERR "[DESTROY][WRK] Secondary workUnit Deallocated\n");
		memoryRelease(workTask[WORK_MASTER_SECONDARY].workUnit, 1, sizeof(struct work_struct));
    }

    printk(KERN_ERR "[DESTROY][WRK] Work unit :: masterTransferSecondary\n");
}

static void scheduleTimer_WorkDestroy(void)
{
    cancel_work_sync(workTask[WORK_SCHEDULE_TIMER].workUnit);
    if (workTask[WORK_SCHEDULE_TIMER].workQueue)
    {
        flush_workqueue(workTask[WORK_SCHEDULE_TIMER].workQueue);
        destroy_workqueue(workTask[WORK_SCHEDULE_TIMER].workQueue);
        workTask[WORK_SCHEDULE_TIMER].workQueue = NULL;
    }

    if (workTask[WORK_SCHEDULE_TIMER].workUnit)
    {
        printk(KERN_ERR "[DESTROY][WRK] Secondary workUnit Deallocated\n");
        memoryRelease(workTask[WORK_SCHEDULE_TIMER].workUnit, 1, sizeof(struct work_struct));
    }

    printk(KERN_ERR "[DESTROY][WRK] Work unit :: scheduleTimer\n");
}

void spiWorkInit(void)
{
	masterTransferPrimary_WorkInit();
	masterTransferSecondary_WorkInit();
	scheduleTimer_WorkInit();
	printk(KERN_ERR "[INIT][WRK] Kernel workflow Created\n");
}

void spiWorkDestroy(void)
{
	masterTransferPrimary_WorkDestroy();
	masterTransferSecondary_WorkDestroy();
	scheduleTimer_WorkDestroy();
	printk(KERN_ERR "[DESTROY][WRK] Kernel workflow destroyed\n");
}
