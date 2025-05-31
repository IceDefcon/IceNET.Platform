/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#ifndef KERNEL_COMMS_H
#define KERNEL_COMMS_H

#include <linux/fs.h>       // Include for file_operations struct
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>  // Include for copy_to_user function
#include <linux/mutex.h>    // Include for mutex opearations
#include <linux/device.h>   // Include for class_create

#define MAIN_COMMANDER_BUFFER_SIZE 32

typedef struct
{
    uint8_t* RxData;
    uint8_t* TxData;
} DmaTransferType;

typedef struct
{
    int majorNumber;
    struct class *deviceClass;
    struct device *nodeDevice;
    int openCount;
    struct mutex deviceMutex;
    size_t transferSize;
    int tryLock;
    DmaTransferType io_transfer;
    struct file_operations fops;
    char* name;
    char* nameClass;
    char unlockTimer;
    bool wakeUpDevice;
}charDeviceData;

void mainCommanderInit(void);
void mainCommanderDestroy(void);

#endif // KERNEL_COMMS_H
