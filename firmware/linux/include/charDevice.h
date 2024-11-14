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

#define  INPUT_DEVICE  "KernelInput"
#define  INPUT_CLASS   "KernelInputClass"
#define  OUTPUT_DEVICE "KernelOutput"
#define  OUTPUT_CLASS  "KernelOutputClass"
#define  WATCHDOG_DEVICE "FpgaWatchdog"
#define  WATCHDOG_CLASS  "FpgaWatchdogClass"

#define IO_BUFFER_SIZE TRANSFER_BUFFER_SIZE

typedef struct
{
    int majorNumber;
    struct class *deviceClass;
    struct device *nodeDevice;
    int openCount;
    struct mutex device_mutex;
    struct mutex read_Mutex;
    int tryLock;
    DataTransfer io_transfer;
    struct file_operations fops;
}charDeviceData;

typedef enum
{
    DEVICE_INPUT,
    DEVICE_OUTPUT,
    DEVICE_WATCHDOG,
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

/* GET */ charDeviceData* getCharDevice(void);
/* GET */ DataTransfer* getCharDeviceTransfer(charDeviceType charDevice);
/* CTRL */ void charDeviceMutexCtrl(charDeviceType charDevice, MutexCtrlType mutexCtrl);

/* INIT */ void charDeviceInit(void);
/* EXIT */ void charDeviceDestroy(void);

#endif // CHAR_DEVICE_H

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// INPUT Mutex
//
// No additional Lock/Unlock operations
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//           inputOpen
//               |
//               |
//               |
//               V
//            tryLock -----------> ERROR
//               |         NO
//               |
//               | YES
//               |
//               |
//               V
//              Lock ---------------------------> inputClose -----------> Unlock -----------> Destroy
//
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// OUTPUT Mutex
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//           outputOpen
//               |
//               |
//               |
//               V
//            tryLock -----------> ERROR
//               |         NO
//               |
//               | YES
//               |
//               |
//               V
//              Lock ---------------------------> outputClose ---------> Unlock -----------> Destroy
//               Λ                      |
//               |                      |
//               |                      |
//               |                      |         [Preamble Error]
//               |                      |-------> transferFpgaInput -----> Unlock -------------------->|
//               |                      |                                                              |
//               |                      |                                                              |
//               |                      |                                                              |
//               |                      |          [0xDE, 0xAD]                                        |
//               |                      |-------> killApplication ---------> Unlock ----------->|      |
//               |                      |                                                       |      |
//               |                      |                                                       |      |
//               |                      |                                                       |      |
//               |                      |       [Indicator Error]                               |      |
//               |                      |-------> watchdogThread -----------> Unlock --->|      |      |
//               |                      |                                                |      |      |
//               |                      |                                                |      |      |
//               |                      |                                                |      |      |
//               |                      |           [Dma Transfer]                       |      |      |
//               |       ---=[ Run ]=---|-------> transferFpgaOutput ---------> Unlock   |      |      |
//               |                                                                |      |      |      |
//               |                                                                |      |      |      |
//               |                                                                |      |      |      |
//               |                                                                |      |      |      |
//               |                      Waiting for Unlock                        V      V      V      V
//           outputRead <-------------------------------------------------------------------------------------------------
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WATCHDOG Mutex
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//           watchdogOpen
//               |
//               |
//               |
//               V
//            tryLock -----------> ERROR
//               |         NO
//               |
//               | YES
//               |
//               |
//               V
//              Lock ---------------------------> watchdogClose -----------> Unlock -----------> Destroy
//               Λ                      |
//               |                      |
//               |                      |
//               |                      |         [0xDE, 0xAD]
//               |                      |-------> killApplication -----------> Unlock -------------->|
//               |                      |                                                            |
//               |                      |                                                            |
//               |                      |                                                            |
//               |                      |         [Every 500ms]                                      |
//               |       ---=[ Run ]=---|-------> watchdogThread -------------> Unlock               |
//               |                                                                |                  |
//               |                                                                |                  |
//               |                                                                |                  |
//               |                                                                |                  |
//               |                      Waiting for Unlock                        V                  V
//           watchdogRead <---------------------------------------------------------------------------
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
