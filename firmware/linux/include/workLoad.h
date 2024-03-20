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
struct work_struct fpga_work;
struct work_struct kernel_work;
struct workqueue_struct *fpga_wq;
struct workqueue_struct *kernel_wq;



#endif // WORK_LOAD_H
