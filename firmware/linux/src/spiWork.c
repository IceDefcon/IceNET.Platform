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

/* WORK QUEUE */ static struct workqueue_struct *transferFromCharDevice_wq;
/* WORK QUEUE */ static struct workqueue_struct* interruptFromFpga_wq;
/* WORK QUEUE */ static struct workqueue_struct* killApplication_wq;
/* WORK */ static struct work_struct transferFromCharDevice_work;
/* WORK */ static struct work_struct interruptFromFpga_work;
/* WORK */ static struct work_struct killApplication_work;

/* GET WORK QUEUE*/ struct workqueue_struct* get_transferFromCharDevice_wq(void) 
{
    return transferFromCharDevice_wq;
}
/* GET WORK QUEUE */ struct workqueue_struct* get_interruptFromFpga_wq(void)
{
	return interruptFromFpga_wq;
}
/* GET WORK QUEUE */ struct workqueue_struct* get_killApplication_wq(void)
{
	return killApplication_wq;
}
/* GET WORK */ struct work_struct* get_transferFromCharDevice_work(void) 
{
    return &transferFromCharDevice_work;
}
/* GET WORK */ struct work_struct* get_interruptFromFpga_work(void)
{
	return &interruptFromFpga_work;
}
/* GET WORK */ struct work_struct* get_killApplication_work(void)
{
	return &killApplication_work;
}

static void interruptFromFpga_WorkInit(void)
{
	INIT_WORK(get_interruptFromFpga_work(), interruptFromFpga);
	interruptFromFpga_wq = create_singlethread_workqueue("interruptFromFpga_workqueue");
	if (!interruptFromFpga_wq) 
	{
	    printk(KERN_ERR "[INIT][WRK] Failed to initialise single thread workqueue for interruptFromFpga: -ENOMEM\n");
	}
	else
	{
	    printk(KERN_ERR "[INIT][WRK] Create single thread workqueue for interruptFromFpga\n");
	}
}

static void transferFromCharDevice_WorkInit(void)
{
	INIT_WORK(get_transferFromCharDevice_work(), transferFromCharDevice);
	transferFromCharDevice_wq = create_singlethread_workqueue("transferFromCharDevice_workqueue");
	if (!transferFromCharDevice_wq) 
	{
	    printk(KERN_ERR "[INIT][WRK] Failed to initialise single thread workqueue for transferFromCharDevice: -ENOMEM\n");
	}
	else
	{
		printk(KERN_ERR "[INIT][WRK] Create single thread workqueue for transferFromCharDevice\n");
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

static void interruptFromFpga_WorkDestroy(void)
{
    cancel_work_sync(get_interruptFromFpga_work());
    if (interruptFromFpga_wq) 
    {
        flush_workqueue(interruptFromFpga_wq);
        destroy_workqueue(interruptFromFpga_wq);
        interruptFromFpga_wq = NULL;
    }
}

static void transferFromCharDevice_WorkDestroy(void)
{
    cancel_work_sync(get_transferFromCharDevice_work());
    if (transferFromCharDevice_wq) 
    {
        flush_workqueue(transferFromCharDevice_wq);
        destroy_workqueue(transferFromCharDevice_wq);
        transferFromCharDevice_wq = NULL;
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
	interruptFromFpga_WorkInit();
	transferFromCharDevice_WorkInit();
	killApplication_WorkInit();
}

void spiWorkDestroy(void)
{
	interruptFromFpga_WorkDestroy();
	transferFromCharDevice_WorkDestroy();
	killApplication_WorkDestroy();
	printk(KERN_ERR "[DESTROY][WRK] Destroy kernel workflow\n");
}
