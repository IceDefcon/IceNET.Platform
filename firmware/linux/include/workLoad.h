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

/* GET */
struct work_struct* get_fpga_work(void);
struct work_struct* get_kernel_work(void);
struct workqueue_struct* get_fpga_wq(void);
struct workqueue_struct* get_kernel_wq(void);

/* SET */
void set_fpga_wq(struct workqueue_struct *wq);
void set_kernel_wq(struct workqueue_struct *wq);
void set_fpga_work(struct work_struct *work);
void set_kernel_work(struct work_struct *work);

#endif // WORK_LOAD_H
