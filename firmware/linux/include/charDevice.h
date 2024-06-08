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

/* GET TRANSFER RX DATA */ DataTransfer* charDevice_getRxData(void);

void charDeviceInit(void);
void charDeviceDestroy(void);

#endif // CHAR_DEVICE_H