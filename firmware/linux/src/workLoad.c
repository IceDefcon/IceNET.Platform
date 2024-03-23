/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include "workLoad.h"


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

/* BASE */ struct work_struct fpga_work;
/* GET */ struct work_struct* get_fpga_work(void) 
{
    return &fpga_work;
}
/* SET */ void set_fpga_work(struct work_struct *work) 
{
    fpga_work = *work;
}



// static struct work_struct ice_kernel_work;
// static struct workqueue_struct* ice_kernel_wq;

// struct workqueue_struct* get_kernel_wq(void)
// {
// 	return ice_kernel_wq;
// }

// struct work_struct* get_kernel_work(void)
// {
// 	return &ice_kernel_work;
// }

// void set_kernel_wq(struct workqueue_struct *wq) 
// {
// 	ice_kernel_wq = wq;
// }

// void set_kernel_work(struct work_struct *work) 
// {
// 	ice_kernel_work = *work;
// }
