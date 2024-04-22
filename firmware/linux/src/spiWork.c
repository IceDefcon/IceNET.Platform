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

/* BASE */ static struct workqueue_struct *signalFromCharDevice_wq;
/* BASE */ static struct work_struct signalFromCharDevice_work;
/* BASE */ static struct workqueue_struct* interruptFromFpga_wq;
/* BASE */ static struct work_struct interruptFromFpga_work;

/* GET */ struct workqueue_struct* get_signalFromCharDevice_wq(void) 
{
    return signalFromCharDevice_wq;
}
/* GET */ struct workqueue_struct* get_interruptFromFpga_wq(void)
{
	return interruptFromFpga_wq;
}
/* GET */ struct work_struct* get_signalFromCharDevice_work(void) 
{
    return &signalFromCharDevice_work;
}
/* GET */ struct work_struct* get_interruptFromFpga_work(void)
{
	return &interruptFromFpga_work;
}

static void interruptFromFpga_WorkInit(void)
{
	INIT_WORK(get_kernel_work(), interruptFromFpga);
	interruptFromFpga_wq = create_singlethread_workqueue("kernel_workqueue");
	if (!interruptFromFpga_wq) 
	{
	    printk(KERN_ERR "[FPGA][WRK] Failed to create kernel workqueue: -ENOMEM\n");
	}
}

static void signalFromCharDevice_WorkInit(void)
{
	INIT_WORK(get_signalFromCharDevice_work(), signalFromCharDevice);
	signalFromCharDevice_wq = create_singlethread_workqueue("fpga_workqueue");
	if (!signalFromCharDevice_wq) 
	{
	    printk(KERN_ERR "[FPGA][WRK] Failed to create fpga workqueue: -ENOMEM\n");
	}
}

static void interruptFromFpga_WorkDestroy(void)
{
    cancel_work_sync(get_kernel_work());
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

void spiWorkInit(void)
{
	interruptFromFpga_WorkInit();
	signalFromCharDevice_WorkInit();
}

void spiWorkDestroy(void)
{
	interruptFromFpga_WorkDestroy();
	signalFromCharDevice_WorkDestroy();
}
