/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
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
    struct mutex deviceMutex;
    bool isLocked;
    size_t transferSize;
    int tryLock;
    DmaTransferType io_transfer;
    struct file_operations fops;
    char* name;
    char* nameClass;;
}charDeviceData;

typedef enum
{
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

/* GET */ uint8_t getCustomDmaSize(void);
/* GET */ charDeviceData* getCharDevice(void);
/* GET */ DmaTransferType* getCharDeviceTransfer(charDeviceType charDevice);
/* MUTEX */ void charDeviceMutexCtrl(charDeviceType charDevice, MutexCtrlType mutexCtrl);
/* FLAG */ void charDeviceLockCtrl(charDeviceType charDevice, CtrlType Ctrl);

/* INIT */ void charDeviceInit(void);
/* EXIT */ void charDeviceDestroy(void);

#endif // CHAR_DEVICE_H
