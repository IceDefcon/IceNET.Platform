/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */

#include "spiDmaCtrl.h"
#include "spiWork.h"

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

	[WORK_CONFIG_FPGA] =
	{
		.workQueue = NULL,
		.workUnit = NULL,
	},
};

/* GET WORK QUEUE*/ struct workqueue_struct* get_masterTransferPrimary_wq(void){return workTask[WORK_MASTER_PRIMARY].workQueue;}
/* GET WORK QUEUE */ struct workqueue_struct* get_masterTransferSecondary_wq(void){return workTask[WORK_MASTER_SECONDARY].workQueue;}

/* GET WORK */ struct work_struct* get_masterTransferPrimary_work(void){return workTask[WORK_MASTER_PRIMARY].workUnit;}
/* GET WORK */ struct work_struct* get_masterTransferSecondary_work(void){return workTask[WORK_MASTER_SECONDARY].workUnit;}

static void masterTransferPrimary_WorkInit(void)
{
    printk(KERN_ERR "[INIT][WRK] masterTransferPrimary :: Init work unit\n");
    workTask[WORK_MASTER_PRIMARY].workUnit = kmalloc(sizeof(struct work_struct), GFP_KERNEL);
    if (!workTask[WORK_MASTER_PRIMARY].workUnit)
    {
        printk(KERN_ERR "[INIT][WRK] Failed to allocate memory for masterTransferPrimary work unit: -ENOMEM\n");
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
    workTask[WORK_MASTER_SECONDARY].workUnit = kmalloc(sizeof(struct work_struct), GFP_KERNEL);
    if (!workTask[WORK_MASTER_SECONDARY].workUnit)
    {
        printk(KERN_ERR "[INIT][WRK] Failed to allocate memory for masterTransferSecondary work unit: -ENOMEM\n");
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

static void masterTransferPrimary_WorkDestroy(void)
{
    cancel_work_sync(workTask[WORK_MASTER_PRIMARY].workUnit);
    if (workTask[WORK_MASTER_PRIMARY].workQueue)
    {
        flush_workqueue(workTask[WORK_MASTER_PRIMARY].workQueue);
        destroy_workqueue(workTask[WORK_MASTER_PRIMARY].workQueue);
        workTask[WORK_MASTER_PRIMARY].workQueue = NULL;
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
    printk(KERN_ERR "[DESTROY][WRK] Work unit :: masterTransferSecondary\n");
}

void spiWorkInit(void)
{
	masterTransferPrimary_WorkInit();
	masterTransferSecondary_WorkInit();
	printk(KERN_ERR "[INIT][WRK] Kernel workflow Created\n");
}

void spiWorkDestroy(void)
{
	masterTransferPrimary_WorkDestroy();
	masterTransferSecondary_WorkDestroy();
	printk(KERN_ERR "[DESTROY][WRK] Kernel workflow destroyed\n");
}
