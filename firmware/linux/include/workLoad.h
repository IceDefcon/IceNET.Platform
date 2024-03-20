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
struct work_struct* get_fpga_work();
struct work_struct* get_kernel_work();
struct workqueue_struct* get_fpga_wq();
struct workqueue_struct* get_kernel_wq();



#endif // WORK_LOAD_H
