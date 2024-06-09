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

/* GET WORK QUEUE */ struct workqueue_struct* get_transferFromCharDevice_wq(void);
/* GET WORK QUEUE */ struct workqueue_struct* get_interruptFromFpga_wq(void);
/* GET WORK QUEUE */ struct workqueue_struct* get_feedbackTransferFromFPGA_wq(void);
/* GET WORK */ struct work_struct* get_transferFromCharDevice_work(void);
/* GET WORK */ struct work_struct* get_interruptFromFpga_work(void);
/* GET WORK */ struct work_struct* get_feedbackTransferFromFPGA_work(void);

/* INIT */ void spiWorkInit(void);
/* DESTROY */ void spiWorkDestroy(void);

#endif // WORK_LOAD_H
