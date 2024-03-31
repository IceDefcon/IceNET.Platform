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

/* FPGA WORK QUEUE */
/* BASE */ struct workqueue_struct *fpga_wq;
/* GET */ struct workqueue_struct* get_fpga_wq(void) 
{
    return fpga_wq;
}
/* SET */ void set_fpga_wq(struct workqueue_struct *wq) 
{
    fpga_wq = wq;
}

/* FPGA WORK */
/* BASE */ struct work_struct fpga_work;
/* GET */ struct work_struct* get_fpga_work(void) 
{
    return &fpga_work;
}

/* KERNEL WORK QUEUE */
/* BASE */ static struct workqueue_struct* kernel_wq;
/* GET */ struct workqueue_struct* get_kernel_wq(void)
{
	return kernel_wq;
}
/* SET */ void set_kernel_wq(struct workqueue_struct *wq) 
{
	kernel_wq = wq;
}

/* KERNEL WORK */
/* BASE */ static struct work_struct kernel_work;
/* GET */ struct work_struct* get_kernel_work(void)
{
	return &kernel_work;
}

int kernelWorkInit(void)
{
	INIT_WORK(get_kernel_work(), spi_kernel_execute);
	set_kernel_wq(create_singlethread_workqueue("kernel_workqueue"));
	if (!get_kernel_wq()) 
	{
	    printk(KERN_ERR "[FPGA][WRK] Failed to create kernel workqueue\n");
	    return -ENOMEM;
	}
}

int fpgaWorkInit(void)
{
	INIT_WORK(get_fpga_work(), spi_fpga_command);
	set_fpga_wq(create_singlethread_workqueue("fpga_workqueue"));
	if (!get_fpga_wq()) 
	{
	    printk(KERN_ERR "[FPGA][WRK] Failed to create fpga workqueue\n");
	    return -ENOMEM;
	}
}