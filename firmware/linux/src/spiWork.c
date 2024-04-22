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

/* BASE */ static struct workqueue_struct *fpga_wq;
/* BASE */ static struct work_struct fpga_work;
/* BASE */ static struct workqueue_struct* kernel_wq;
/* BASE */ static struct work_struct kernel_work;

/* GET */ struct workqueue_struct* get_fpga_wq(void) 
{
    return fpga_wq;
}
/* GET */ struct workqueue_struct* get_kernel_wq(void)
{
	return kernel_wq;
}
/* GET */ struct work_struct* get_fpga_work(void) 
{
    return &fpga_work;
}
/* GET */ struct work_struct* get_kernel_work(void)
{
	return &kernel_work;
}

static void interruptFromFpga_WorkInit(void)
{
	INIT_WORK(get_kernel_work(), interruptFromFpga);
	kernel_wq = create_singlethread_workqueue("kernel_workqueue");
	if (!kernel_wq) 
	{
	    printk(KERN_ERR "[FPGA][WRK] Failed to create kernel workqueue: -ENOMEM\n");
	}
}

static void signalFromCharDevice_WorkInit(void)
{
	INIT_WORK(get_fpga_work(), signalFromCharDevice);
	fpga_wq = create_singlethread_workqueue("fpga_workqueue");
	if (!fpga_wq) 
	{
	    printk(KERN_ERR "[FPGA][WRK] Failed to create fpga workqueue: -ENOMEM\n");
	}
}

static void interruptFromFpga_WorkDestroy(void)
{
    cancel_work_sync(get_kernel_work());
    if (kernel_wq) {
        flush_workqueue(kernel_wq);
        destroy_workqueue(kernel_wq);
        kernel_wq = NULL;
    }
}

static void signalFromCharDevice_WorkDestroy(void)
{
    cancel_work_sync(get_fpga_work());
    if (fpga_wq) {
        flush_workqueue(fpga_wq);
        destroy_workqueue(fpga_wq);
        fpga_wq = NULL;
    }
}

void spiWorkInit(void)
{
	interruptFromFpga_WorkInit()
	signalFromCharDevice_WorkInit();
}

void spiWorkDestroy(void)
{
	interruptFromFpga_WorkDestroy();
	signalFromCharDevice_WorkDestroy();
}
