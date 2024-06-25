/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#ifndef CHAR_DEVICE_H
#define CHAR_DEVICE_H

#include "types.h"

//////////////////////
//                  //
//                  //
//                  //
//    [C] Device    //
//                  //
//                  //
//                  //
//////////////////////

typedef enum
{
    KERNEL_INPUT,
    KERNEL_OUTPUT,
    KERNEL_FOPS_AMOUNT
}deviceTYPE;

/* GET */ DataTransfer* get_KernelInputTransfer(void);
/* SET */ void set_fpgaFeedbackTransfer(const DataTransfer* transferData);
/* SET */ void set_killApplication(const DataTransfer* transferData);

void charDeviceInit(void);
void charDeviceDestroy(void);

#endif // CHAR_DEVICE_H