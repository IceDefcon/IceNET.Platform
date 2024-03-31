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
/* SET */ void set_fpga_wq(struct workqueue_struct *wq);
/* GET */ struct work_struct* get_fpga_work(void);

/* GET */ struct workqueue_struct* get_kernel_wq(void);
/* SET */ void set_kernel_wq(struct workqueue_struct *wq);
/* GET */ struct work_struct* get_kernel_work(void);

int kernelWorkInit(void);
int fpgaWorkInit(void);


#endif // WORK_LOAD_H
