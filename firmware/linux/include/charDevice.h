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

#define  INPUT_DEVICE  "KernelInput"
#define  INPUT_CLASS   "KernelInputClass"
#define  OUTPUT_DEVICE "KernelOutput"
#define  OUTPUT_CLASS  "KernelOutputClass"

typedef struct
{
    int majorNumber;
    struct class *deviceClass;
    struct device *nodeDevice;
    int openCount;
    struct mutex io_mutex;
    DataTransfer io_transfer;
}charDeviceData;

typedef enum
{
    DEVICE_INPUT,
    DEVICE_OUTPUT,
    DEVICE_AMOUNT
}deviceType;

/* GET */ DataTransfer* getKernelInputTransfer(void);
/* SET */ void setFpgaFeedbackTransfer(const DataTransfer* transferData);
/* SET */ void setkillApplicationTransfer(const DataTransfer* transferData);

void charDeviceInit(void);
void charDeviceDestroy(void);

#endif // CHAR_DEVICE_H