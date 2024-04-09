/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include "workLoad.h"
#include "spiEngine.h"

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

void spiKernelWorkInit(void)
{
	INIT_WORK(get_kernel_work(), interruptFromFpgaExecute);
	kernel_wq = create_singlethread_workqueue("kernel_workqueue");
	if (!kernel_wq) 
	{
	    printk(KERN_ERR "[FPGA][WRK] Failed to create kernel workqueue: -ENOMEM\n");
	}
}

void spiFpgaWorkInit(void)
{
	INIT_WORK(get_fpga_work(), signalFromCharDevice);
	fpga_wq = create_singlethread_workqueue("fpga_workqueue");
	if (!fpga_wq) 
	{
	    printk(KERN_ERR "[FPGA][WRK] Failed to create fpga workqueue: -ENOMEM\n");
	}
}

void spiKernelWorkDestroy(void)
{
    cancel_work_sync(get_kernel_work());
    if (kernel_wq) {
        flush_workqueue(kernel_wq);
        destroy_workqueue(kernel_wq);
        kernel_wq = NULL;
    }
}

void spiFpgaWorkDestroy(void)
{
    cancel_work_sync(get_fpga_work());
    if (fpga_wq) {
        flush_workqueue(fpga_wq);
        destroy_workqueue(fpga_wq);
        fpga_wq = NULL;
    }
}