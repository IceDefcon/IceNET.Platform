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

/* INIT */ int kernelWorkInit(void);
/* INIT */ int fpgaWorkInit(void);
/* DESTROY */ void kernelWorkDestroy(void);
/* DESTROY */ void fpgaWorkDestroy(void);

#endif // WORK_LOAD_H
