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

/* WORK QUEUE */ static struct workqueue_struct *secondFromCharDevice_wq;
/* WORK QUEUE */ static struct workqueue_struct *mainFromCharDevice_wq;
/* WORK QUEUE */ static struct workqueue_struct* interruptFromFpga_wq;
/* WORK */ static struct work_struct secondFromCharDevice_work;
/* WORK */ static struct work_struct mainFromCharDevice_work;
/* WORK */ static struct work_struct interruptFromFpga_work;

/* GET WORK QUEUE*/ struct workqueue_struct* get_secondFromCharDevice_wq(void) 
{
    return secondFromCharDevice_wq;
}
/* GET WORK QUEUE*/ struct workqueue_struct* get_mainFromCharDevice_wq(void) 
{
    return mainFromCharDevice_wq;
}
/* GET WORK QUEUE */ struct workqueue_struct* get_interruptFromFpga_wq(void)
{
	return interruptFromFpga_wq;
}
/* GET WORK */ struct work_struct* get_secondFromCharDevice_work(void) 
{
    return &secondFromCharDevice_work;
}
/* GET WORK */ struct work_struct* get_mainFromCharDevice_work(void) 
{
    return &mainFromCharDevice_work;
}
/* GET WORK */ struct work_struct* get_interruptFromFpga_work(void)
{
	return &interruptFromFpga_work;
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

static void mainFromCharDevice_WorkInit(void)
{
	INIT_WORK(get_mainFromCharDevice_work(), mainFromCharDevice);
	mainFromCharDevice_wq = create_singlethread_workqueue("mainFromCharDevice_workqueue");
	if (!mainFromCharDevice_wq) 
	{
	    printk(KERN_ERR "[INIT][WRK] Failed to initialise single thread workqueue for mainFromCharDevice: -ENOMEM\n");
	}
	else
	{
		printk(KERN_ERR "[INIT][WRK] Create single thread workqueue for mainFromCharDevice\n");
	}
}

static void secondFromCharDevice_WorkInit(void)
{
	INIT_WORK(get_secondFromCharDevice_work(), secondFromCharDevice);
	secondFromCharDevice_wq = create_singlethread_workqueue("secondFromCharDevice_workqueue");
	if (!secondFromCharDevice_wq) 
	{
	    printk(KERN_ERR "[INIT][WRK] Failed to initialise single thread workqueue for secondFromCharDevice: -ENOMEM\n");
	}
	else
	{
		printk(KERN_ERR "[INIT][WRK] Create single thread workqueue for secondFromCharDevice\n");
	}
}

static void interruptFromFpga_WorkDestroy(void)
{
    cancel_work_sync(get_interruptFromFpga_work());
    if (interruptFromFpga_wq) {
        flush_workqueue(interruptFromFpga_wq);
        destroy_workqueue(interruptFromFpga_wq);
        interruptFromFpga_wq = NULL;
    }
}

static void mainFromCharDevice_WorkDestroy(void)
{
    cancel_work_sync(get_mainFromCharDevice_work());
    if (mainFromCharDevice_wq) {
        flush_workqueue(mainFromCharDevice_wq);
        destroy_workqueue(mainFromCharDevice_wq);
        mainFromCharDevice_wq = NULL;
    }
}

static void secondFromCharDevice_WorkDestroy(void)
{
    cancel_work_sync(get_secondFromCharDevice_work());
    if (secondFromCharDevice_wq) {
        flush_workqueue(secondFromCharDevice_wq);
        destroy_workqueue(secondFromCharDevice_wq);
        secondFromCharDevice_wq = NULL;
    }
}

void spiWorkInit(void)
{
	interruptFromFpga_WorkInit();
	mainFromCharDevice_WorkInit();
	secondFromCharDevice_WorkInit();
}

void spiWorkDestroy(void)
{
	interruptFromFpga_WorkDestroy();
	mainFromCharDevice_WorkDestroy();
	secondFromCharDevice_WorkDestroy();
	printk(KERN_ERR "[DESTROY][WRK] Destroy kernel workflow\n");
}
