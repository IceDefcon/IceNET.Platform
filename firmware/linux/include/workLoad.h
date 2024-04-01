/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#ifndef WORK_LOAD_H
#define WORK_LOAD_H

#include <linux/workqueue.h>

//////////////////////
//                  //
//                  //
//                  //
//   [W] Workload   //
//                  //
//                  //
//                  //
//////////////////////

/* GET */ struct workqueue_struct* get_fpga_wq(void);
/* GET */ struct work_struct* get_fpga_work(void);
/* GET */ struct workqueue_struct* get_kernel_wq(void);
/* GET */ struct work_struct* get_kernel_work(void);

/* INIT */ void spiKernelWorkInit(void);
/* INIT */ void spiFpgaWorkInit(void);
/* DESTROY */ void spiKernelWorkDestroy(void);
/* DESTROY */ void spiFpgaWorkDestroy(void);

#endif // WORK_LOAD_H
