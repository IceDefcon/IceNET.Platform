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

typedef enum
{
	WORK_FPGA_INPUT,
	WORK_FPGA_OUTPUT,
	WORK_KILL_APPLICATION,
	WORK_CONFIG_FPGA,
	WORK_AMOUNT
}workType;

typedef struct
{
	struct workqueue_struct *workQueue;
	struct work_struct *workUnit;
}workTaskData;

/* GET WORK QUEUE */ struct workqueue_struct* get_transferFpgaInput_wq(void);
/* GET WORK QUEUE */ struct workqueue_struct* get_transferFpgaOutput_wq(void);
/* GET WORK QUEUE */ struct workqueue_struct* get_killApplication_wq(void);
/* GET WORK QUEUE */ struct workqueue_struct* get_configFpga_wq(void);
/* GET WORK */ struct work_struct* get_transferFpgaInput_work(void);
/* GET WORK */ struct work_struct* get_transferFpgaOutput_work(void);
/* GET WORK */ struct work_struct* get_killApplication_work(void);
/* GET WORK */ struct work_struct* get_configFpga_work(void);

/* INIT */ void spiWorkInit(void);
/* DESTROY */ void spiWorkDestroy(void);

#endif // WORK_LOAD_H
