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

/* WORK QUEUE */ static struct workqueue_struct *testFromCharDevice_wq;
/* WORK QUEUE */ static struct workqueue_struct *signalFromCharDevice_wq;
/* WORK QUEUE */ static struct workqueue_struct* interruptFromFpga_wq;
/* WORK */ static struct work_struct testFromCharDevice_work;
/* WORK */ static struct work_struct signalFromCharDevice_work;
/* WORK */ static struct work_struct interruptFromFpga_work;

/* GET WORK QUEUE*/ struct workqueue_struct* get_testFromCharDevice_wq(void) 
{
    return testFromCharDevice_wq;
}
/* GET WORK QUEUE*/ struct workqueue_struct* get_signalFromCharDevice_wq(void) 
{
    return signalFromCharDevice_wq;
}
/* GET WORK QUEUE */ struct workqueue_struct* get_interruptFromFpga_wq(void)
{
	return interruptFromFpga_wq;
}
/* GET WORK */ struct work_struct* get_testFromCharDevice_work(void) 
{
    return &testFromCharDevice_work;
}
/* GET WORK */ struct work_struct* get_signalFromCharDevice_work(void) 
{
    return &signalFromCharDevice_work;
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
	    printk(KERN_ERR "[INIT][WRK] Failed to create workqueue for interrupt from FPGA: -ENOMEM\n");
	}
}

static void signalFromCharDevice_WorkInit(void)
{
	INIT_WORK(get_signalFromCharDevice_work(), signalFromCharDevice);
	signalFromCharDevice_wq = create_singlethread_workqueue("signalFromCharDevice_workqueue");
	if (!signalFromCharDevice_wq) 
	{
	    printk(KERN_ERR "[INIT][WRK] Failed to create workqueue for signal From Char Device: -ENOMEM\n");
	}
}

static void testFromCharDevice_WorkInit(void)
{
	INIT_WORK(get_testFromCharDevice_work(), testFromCharDevice);
	testFromCharDevice_wq = create_singlethread_workqueue("testFromCharDevice_workqueue");
	if (!testFromCharDevice_wq) 
	{
	    printk(KERN_ERR "[INIT][WRK] Failed to create workqueue for test From Char Device: -ENOMEM\n");
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

static void signalFromCharDevice_WorkDestroy(void)
{
    cancel_work_sync(get_signalFromCharDevice_work());
    if (signalFromCharDevice_wq) {
        flush_workqueue(signalFromCharDevice_wq);
        destroy_workqueue(signalFromCharDevice_wq);
        signalFromCharDevice_wq = NULL;
    }
}

static void testFromCharDevice_WorkDestroy(void)
{
    cancel_work_sync(get_testFromCharDevice_work());
    if (testFromCharDevice_wq) {
        flush_workqueue(testFromCharDevice_wq);
        destroy_workqueue(testFromCharDevice_wq);
        testFromCharDevice_wq = NULL;
    }
}

void spiWorkInit(void)
{
	interruptFromFpga_WorkInit();
	signalFromCharDevice_WorkInit();
	testFromCharDevice_WorkInit();
}

void spiWorkDestroy(void)
{
	interruptFromFpga_WorkDestroy();
	signalFromCharDevice_WorkDestroy();
	testFromCharDevice_WorkDestroy();
}
