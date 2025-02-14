/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
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

typedef enum
{
	WORK_MASTER_PRIMARY,
	WORK_MASTER_SECONDARY,
	WORK_CONFIG_FPGA,
	WORK_AMOUNT
}workType;

typedef struct
{
	struct workqueue_struct *workQueue;
	struct work_struct *workUnit;
}workTaskData;

/* GET WORK QUEUE */ struct workqueue_struct* get_masterTransferPrimary_wq(void);
/* GET WORK QUEUE */ struct workqueue_struct* get_masterTransferSecondary_wq(void);
/* GET WORK */ struct work_struct* get_masterTransferPrimary_work(void);
/* GET WORK */ struct work_struct* get_masterTransferSecondary_work(void);

/* INIT */ void spiWorkInit(void);
/* DESTROY */ void spiWorkDestroy(void);

#endif // WORK_LOAD_H
