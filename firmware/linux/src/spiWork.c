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
	[WORK_FPGA_INPUT] =
	{
		.workQueue = NULL,
		.workUnit = NULL,
	},

	[WORK_FPGA_OUTPUT] =
	{
		.workQueue = NULL,
		.workUnit = NULL,
	},

	[WORK_KILL_APPLICATION] =
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

/* GET WORK QUEUE*/ struct workqueue_struct* get_transferFpgaInput_wq(void){return workTask[WORK_FPGA_INPUT].workQueue;}
/* GET WORK QUEUE */ struct workqueue_struct* get_transferFpgaOutput_wq(void){return workTask[WORK_FPGA_OUTPUT].workQueue;}
/* GET WORK QUEUE */ struct workqueue_struct* get_killApplication_wq(void){return workTask[WORK_KILL_APPLICATION].workQueue;}

/* GET WORK */ struct work_struct* get_transferFpgaInput_work(void){return workTask[WORK_FPGA_INPUT].workUnit;}
/* GET WORK */ struct work_struct* get_transferFpgaOutput_work(void){return workTask[WORK_FPGA_OUTPUT].workUnit;}
/* GET WORK */ struct work_struct* get_killApplication_work(void){return workTask[WORK_KILL_APPLICATION].workUnit;}

static void transferFpgaInput_WorkInit(void)
{
    printk(KERN_ERR "[INIT][WRK] transferFpgaInput :: Init work unit\n");
    workTask[WORK_FPGA_INPUT].workUnit = kmalloc(sizeof(struct work_struct), GFP_KERNEL);
    if (!workTask[WORK_FPGA_INPUT].workUnit)
    {
        printk(KERN_ERR "[INIT][WRK] Failed to allocate memory for transferFpgaInput work unit: -ENOMEM\n");
    }
	else
	{
		printk(KERN_ERR "[INIT][WRK] transferFpgaInput :: Memory allocattion successfully\n");
	}

	INIT_WORK(workTask[WORK_FPGA_INPUT].workUnit, transferFpgaInput);
	workTask[WORK_FPGA_INPUT].workQueue = create_singlethread_workqueue("transferFpgaInput_workqueue");
	if (!workTask[WORK_FPGA_INPUT].workQueue)
	{
	    printk(KERN_ERR "[INIT][WRK] Failed to initialise single thread workqueue for transferFpgaInput: -ENOMEM\n");
	}
	else
	{
		printk(KERN_ERR "[INIT][WRK] transferFpgaInput :: Single thread workqueue created successfully\n");
	}
}

static void transferFpgaOutput_WorkInit(void)
{
    printk(KERN_ERR "[INIT][WRK] transferFpgaOutput :: Init work unit\n");
    workTask[WORK_FPGA_OUTPUT].workUnit = kmalloc(sizeof(struct work_struct), GFP_KERNEL);
    if (!workTask[WORK_FPGA_OUTPUT].workUnit)
    {
        printk(KERN_ERR "[INIT][WRK] Failed to allocate memory for transferFpgaOutput work unit: -ENOMEM\n");
    }
	else
	{
		printk(KERN_ERR "[INIT][WRK] transferFpgaOutput :: Memory allocattion successfully\n");
	}

	INIT_WORK(workTask[WORK_FPGA_OUTPUT].workUnit, transferFpgaOutput);
	workTask[WORK_FPGA_OUTPUT].workQueue = create_singlethread_workqueue("transferFpgaInput_workqueue");
	if (!workTask[WORK_FPGA_OUTPUT].workQueue)
	{
	    printk(KERN_ERR "[INIT][WRK] Failed to initialise single thread workqueue for transferFpgaOutput: -ENOMEM\n");
	}
	else
	{
		printk(KERN_ERR "[INIT][WRK] transferFpgaOutput :: Single thread workqueue created successfully\n");
	}
}

static void killApplication_WorkInit(void)
{
    printk(KERN_ERR "[INIT][WRK] killApplication :: Init work unit\n");
    workTask[WORK_KILL_APPLICATION].workUnit = kmalloc(sizeof(struct work_struct), GFP_KERNEL);
    if (!workTask[WORK_KILL_APPLICATION].workUnit)
    {
        printk(KERN_ERR "[INIT][WRK] Failed to allocate memory for killApplication work unit: -ENOMEM\n");
    }
	else
	{
		printk(KERN_ERR "[INIT][WRK] killApplication :: Memory allocattion successfully\n");
	}

	INIT_WORK(workTask[WORK_KILL_APPLICATION].workUnit, killApplication);
	workTask[WORK_KILL_APPLICATION].workQueue = create_singlethread_workqueue("transferFpgaInput_workqueue");
	if (!workTask[WORK_KILL_APPLICATION].workQueue)
	{
	    printk(KERN_ERR "[INIT][WRK] Failed to initialise single thread workqueue for killApplication: -ENOMEM\n");
	}
	else
	{
		printk(KERN_ERR "[INIT][WRK] killApplication :: Single thread workqueue created successfully\n");
	}
}

static void transferFpgaInput_WorkDestroy(void)
{
    cancel_work_sync(workTask[WORK_FPGA_INPUT].workUnit);
    if (workTask[WORK_FPGA_INPUT].workQueue)
    {
        flush_workqueue(workTask[WORK_FPGA_INPUT].workQueue);
        destroy_workqueue(workTask[WORK_FPGA_INPUT].workQueue);
        workTask[WORK_FPGA_INPUT].workQueue = NULL;
    }
    printk(KERN_ERR "[DESTROY][WRK] Work unit :: transferFpgaInput\n");
}

static void transferFpgaOutput_WorkDestroy(void)
{
    cancel_work_sync(workTask[WORK_FPGA_OUTPUT].workUnit);
    if (workTask[WORK_FPGA_OUTPUT].workQueue)
    {
        flush_workqueue(workTask[WORK_FPGA_OUTPUT].workQueue);
        destroy_workqueue(workTask[WORK_FPGA_OUTPUT].workQueue);
        workTask[WORK_FPGA_OUTPUT].workQueue = NULL;
    }
    printk(KERN_ERR "[DESTROY][WRK] Work unit :: transferFpgaOutput\n");
}

static void killApplication_WorkDestroy(void)
{
    cancel_work_sync(workTask[WORK_KILL_APPLICATION].workUnit);
    if (workTask[WORK_KILL_APPLICATION].workQueue)
    {
        flush_workqueue(workTask[WORK_KILL_APPLICATION].workQueue);
        destroy_workqueue(workTask[WORK_KILL_APPLICATION].workQueue);
        workTask[WORK_KILL_APPLICATION].workQueue = NULL;
    }
    printk(KERN_ERR "[DESTROY][WRK] Work unit :: killApplication\n");
}

void spiWorkInit(void)
{
	transferFpgaInput_WorkInit();
	transferFpgaOutput_WorkInit();
	killApplication_WorkInit();
	printk(KERN_ERR "[INIT][WRK] Kernel workflow Created\n");
}

void spiWorkDestroy(void)
{
	transferFpgaInput_WorkDestroy();
	transferFpgaOutput_WorkDestroy();
	killApplication_WorkDestroy();
	printk(KERN_ERR "[DESTROY][WRK] Kernel workflow destroyed\n");
}
