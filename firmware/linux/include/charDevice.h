/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#ifndef CHAR_DEVICE_H
#define CHAR_DEVICE_H

#include "Types.h"

//////////////////////
//                  //
//                  //
//                  //
//    [C] Device    //
//                  //
//                  //
//                  //
//////////////////////

/* GET Kernel_IN TRANSFER */ DataTransfer* get_Kernel_INTransfer(void);
/* SET FEEDBACK TRANSFER */ void set_fpgaFeedbackTransfer(const DataTransfer* transferData);

void charDeviceInit(void);
void charDeviceDestroy(void);

#endif // CHAR_DEVICE_H