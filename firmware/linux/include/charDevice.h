/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#ifndef CHAR_DEVICE_H
#define CHAR_DEVICE_H

#include <linux/fs.h>       // Include for file_operations struct
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

#define IO_BUFFER_SIZE TRANSFER_BUFFER_SIZE

typedef struct
{
    int majorNumber;
    struct class *deviceClass;
    struct device *nodeDevice;
    int openCount;
    struct mutex read_Mutex;
    bool isLocked;
    int tryLock;
    DataTransfer io_transfer;
    struct file_operations fops;
}charDeviceData;

typedef enum
{
    DEVICE_INPUT,
    DEVICE_OUTPUT,
    DEVICE_WATCHDOG,
    DEVICE_COMMANDER,
    DEVICE_AMOUNT
}charDeviceType;

typedef enum
{
    MUTEX_CTRL_INIT,
    MUTEX_CTRL_LOCK,
    MUTEX_CTRL_TRYLOCK,
    MUTEX_CTRL_UNLOCK,
    MUTEX_CTRL_DESTROY,
    MUTEX_CTRL_AMOUNT
} MutexCtrlType;

typedef enum
{
    CTRL_LOCK,
    CTRL_UNLOCK,
    CTRL_AMOUNT
} CtrlType;

/* GET */ charDeviceData* getCharDevice(void);
/* GET */ DataTransfer* getCharDeviceTransfer(charDeviceType charDevice);
/* MUTEX */ void charDeviceMutexCtrl(charDeviceType charDevice, MutexCtrlType mutexCtrl);
/* FLAG */ void charDeviceLockCtrl(charDeviceType charDevice, CtrlType Ctrl);

/* INIT */ void charDeviceInit(void);
/* EXIT */ void charDeviceDestroy(void);

#endif // CHAR_DEVICE_H
