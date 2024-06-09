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
/* WORK QUEUE */ static struct workqueue_struct* feedbackTransferFromFPGA_wq;
/* WORK */ static struct work_struct transferFromCharDevice_work;
/* WORK */ static struct work_struct interruptFromFpga_work;
/* WORK */ static struct work_struct feedbackTransferFromFPGA_work;

/* GET WORK QUEUE*/ struct workqueue_struct* get_transferFromCharDevice_wq(void) 
{
    return transferFromCharDevice_wq;
}
/* GET WORK QUEUE */ struct workqueue_struct* get_interruptFromFpga_wq(void)
{
	return interruptFromFpga_wq;
}
/* GET WORK QUEUE */ struct workqueue_struct* get_feedbackTransferFromFPGA_wq(void)
{
	return feedbackTransferFromFPGA_wq;
}
/* GET WORK */ struct work_struct* get_transferFromCharDevice_work(void) 
{
    return &transferFromCharDevice_work;
}
/* GET WORK */ struct work_struct* get_interruptFromFpga_work(void)
{
	return &interruptFromFpga_work;
}
/* GET WORK */ struct work_struct* get_feedbackTransferFromFPGA_work(void)
{
	return &feedbackTransferFromFPGA_work;
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

static void feedbackTransferFromFPGA_WorkInit(void)
{
	INIT_WORK(get_feedbackTransferFromFPGA_work(), feedbackTransferFromFPGA);
	feedbackTransferFromFPGA_wq = create_singlethread_workqueue("feedbackTransferFromFPGA_workqueue");
	if (!feedbackTransferFromFPGA_wq) 
	{
	    printk(KERN_ERR "[INIT][WRK] Failed to initialise single thread workqueue for feedbackTransferFromFPGA: -ENOMEM\n");
	}
	else
	{
		printk(KERN_ERR "[INIT][WRK] Create single thread workqueue for feedbackTransferFromFPGA\n");
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

static void feedbackTransferFromFPGA_WorkDestroy(void)
{
    cancel_work_sync(get_feedbackTransferFromFPGA_work());
    if (feedbackTransferFromFPGA_wq) 
    {
        flush_workqueue(feedbackTransferFromFPGA_wq);
        destroy_workqueue(feedbackTransferFromFPGA_wq);
        feedbackTransferFromFPGA_wq = NULL;
    }
}

void spiWorkInit(void)
{
	interruptFromFpga_WorkInit();
	transferFromCharDevice_WorkInit();
	feedbackTransferFromFPGA_WorkInit();
}

void spiWorkDestroy(void)
{
	interruptFromFpga_WorkDestroy();
	transferFromCharDevice_WorkDestroy();
	feedbackTransferFromFPGA_WorkDestroy();
	printk(KERN_ERR "[DESTROY][WRK] Destroy kernel workflow\n");
}
