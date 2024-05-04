/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#ifndef CHAR_DEVICE_H
#define CHAR_DEVICE_H

#include <stdbool.h>
#include <stddef.h>

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

static struct charDevice_DataType /* Transfer data from user-space */
{
    volatile char *RxData;
    volatile char *TxData;
    size_t length;
    bool ready;
} charDevice_Data;

/* GET TRANSFER DATA */ struct charDevice_DataType* charDevice_getRxData(void);

void charDeviceInit(void);
void charDeviceDestroy(void);

#endif // CHAR_DEVICE_H