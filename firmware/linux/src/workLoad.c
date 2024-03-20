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
static struct work_struct fpga_work;
static struct work_struct kernel_work;
static struct workqueue_struct* fpga_wq;
static struct workqueue_struct* kernel_wq;

struct work_struct* get_fpga_work() return &fpga_work;
struct work_struct* get_kernel_work() return &kernel_work;
struct workqueue_struct* get_fpga_wq() return fpga_wq;
struct workqueue_struct* get_kernel_wq() return kernel_wq;