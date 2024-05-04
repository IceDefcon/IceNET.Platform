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

#define CD_OK 0

static struct /* Feedback data to user-space */
{
    char *data;
    size_t length;
    bool ready
} charDevice_TxData;

static dataTransfer charDevice_DataType; 

/* GET TRANSFER DATA */ struct charDevice_DataType* charDevice_getRxData(void);

void charDeviceInit(void);
void charDeviceDestroy(void);

#endif // CHAR_DEVICE_H