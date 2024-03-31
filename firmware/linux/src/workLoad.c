/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include "workLoad.h"
#include "spiFpga.h"

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

int kernelWorkInit(void)
{
	INIT_WORK(get_kernel_work(), spiKernelExecute);
	kernel_wq = create_singlethread_workqueue("kernel_workqueue");
	if (!get_kernel_wq()) 
	{
	    printk(KERN_ERR "[FPGA][WRK] Failed to create kernel workqueue\n");
	    return -ENOMEM;
	}
}

int fpgaWorkInit(void)
{
	INIT_WORK(get_fpga_work(), spiFpgaExecute);
	fpga_wq = create_singlethread_workqueue("fpga_workqueue");
	if (!get_fpga_wq()) 
	{
	    printk(KERN_ERR "[FPGA][WRK] Failed to create fpga workqueue\n");
	    return -ENOMEM;
	}
}

void kernelWorkDestroy(void)
{

    cancel_work_sync(get_kernel_work());
    if (get_kernel_wq()) {
        flush_workqueue(get_kernel_wq());
        destroy_workqueue(get_kernel_wq());
        kernel_wq = NULL;
    }
}

void fpgaWorkDestroy(void)
{

    cancel_work_sync(get_fpga_work());
    if (get_fpga_wq()) {
        flush_workqueue(get_fpga_wq());
        destroy_workqueue(get_fpga_wq());
        fpga_wq = NULL;
    }
}