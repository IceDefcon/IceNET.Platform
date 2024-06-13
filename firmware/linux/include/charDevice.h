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

/* GET inputCOM TRANSFER */ DataTransfer* get_inputCOMTransfer(void);
/* SET FEEDBACK TRANSFER */ void set_fpgaFeedbackTransfer(const DataTransfer* transferData);

void charDeviceInit(void);
void charDeviceDestroy(void);

#endif // CHAR_DEVICE_H