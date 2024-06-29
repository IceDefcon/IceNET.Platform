/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include "spiWork.h"
#include "spiCtrl.h"

//////////////////////
//                  //
//                  //
//                  //
//   [W] Workload   //
//                  //
//                  //
//                  //
//////////////////////

/* WORK QUEUE */ static struct workqueue_struct *transferFpgaInput_wq;
/* WORK QUEUE */ static struct workqueue_struct* transferFpgaOutput_wq;
/* WORK QUEUE */ static struct workqueue_struct* killApplication_wq;
/* WORK */ static struct work_struct transferFpgaInput_work;
/* WORK */ static struct work_struct transferFpgaOutput_work;
/* WORK */ static struct work_struct killApplication_work;

/* GET WORK QUEUE*/ struct workqueue_struct* get_transferFpgaInput_wq(void)
{
    return transferFpgaInput_wq;
}
/* GET WORK QUEUE */ struct workqueue_struct* get_transferFpgaOutput_wq(void)
{
	return transferFpgaOutput_wq;
}
/* GET WORK QUEUE */ struct workqueue_struct* get_killApplication_wq(void)
{
	return killApplication_wq;
}
/* GET WORK */ struct work_struct* get_transferFpgaInput_work(void)
{
    return &transferFpgaInput_work;
}
/* GET WORK */ struct work_struct* get_transferFpgaOutput_work(void)
{
	return &transferFpgaOutput_work;
}
/* GET WORK */ struct work_struct* get_killApplication_work(void)
{
	return &killApplication_work;
}

static void transferFpgaOutput_WorkInit(void)
{
	INIT_WORK(get_transferFpgaOutput_work(), transferFpgaOutput);
	transferFpgaOutput_wq = create_singlethread_workqueue("transferFpgaOutput_workqueue");
	if (!transferFpgaOutput_wq)
	{
	    printk(KERN_ERR "[INIT][WRK] Failed to initialise single thread workqueue for transferFpgaOutput: -ENOMEM\n");
	}
	else
	{
	    printk(KERN_ERR "[INIT][WRK] Create single thread workqueue for transferFpgaOutput\n");
	}
}

static void transferFpgaInput_WorkInit(void)
{
	INIT_WORK(get_transferFpgaInput_work(), transferFpgaInput);
	transferFpgaInput_wq = create_singlethread_workqueue("transferFpgaInput_workqueue");
	if (!transferFpgaInput_wq)
	{
	    printk(KERN_ERR "[INIT][WRK] Failed to initialise single thread workqueue for transferFpgaInput: -ENOMEM\n");
	}
	else
	{
		printk(KERN_ERR "[INIT][WRK] Create single thread workqueue for transferFpgaInput\n");
	}
}

static void killApplication_WorkInit(void)
{
    INIT_WORK(get_killApplication_work(), killApplication);
    killApplication_wq = create_singlethread_workqueue("killApplication_workqueue");
    if (!killApplication_wq) 
    {
        printk(KERN_ERR "[INIT][WRK] Failed to initialise single thread workqueue for killApplication: -ENOMEM\n");
    }
    else
    {
        printk(KERN_ERR "[INIT][WRK] Created single thread workqueue for killApplication\n");
    }
}

static void transferFpgaOutput_WorkDestroy(void)
{
    cancel_work_sync(get_transferFpgaOutput_work());
    if (transferFpgaOutput_wq)
    {
        flush_workqueue(transferFpgaOutput_wq);
        destroy_workqueue(transferFpgaOutput_wq);
        transferFpgaOutput_wq = NULL;
    }
}

static void transferFpgaInput_WorkDestroy(void)
{
    cancel_work_sync(get_transferFpgaInput_work());
    if (transferFpgaInput_wq)
    {
        flush_workqueue(transferFpgaInput_wq);
        destroy_workqueue(transferFpgaInput_wq);
        transferFpgaInput_wq = NULL;
    }
}

static void killApplication_WorkDestroy(void)
{
    cancel_work_sync(get_killApplication_work());
    if (killApplication_wq) 
    {
        flush_workqueue(killApplication_wq);
        destroy_workqueue(killApplication_wq);
        killApplication_wq = NULL;
    }
}

void spiWorkInit(void)
{
	transferFpgaOutput_WorkInit();
	transferFpgaInput_WorkInit();
	killApplication_WorkInit();
}

void spiWorkDestroy(void)
{
	transferFpgaOutput_WorkDestroy();
	transferFpgaInput_WorkDestroy();
	killApplication_WorkDestroy();
	printk(KERN_ERR "[DESTROY][WRK] Destroy kernel workflow\n");
}
