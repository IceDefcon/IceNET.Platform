/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h>  // Include for copy_to_user function
#include <linux/mutex.h>    // Include for mutex opearations
#include <linux/device.h>   // Include for class_create

#include "stateMachine.h"
#include "charDevice.h"
#include "spiWork.h"
#include "memory.h"
#include "types.h"

//////////////////////
//                  //
//                  //
//                  //
//   [ C ] Config   //
//                  //
//                  //
//                  //
//////////////////////

static void charDeviceDataInit(charDeviceType DeviceType)
{
    charDeviceData* pChar = getCharDevice();

    /* Allocate memory */
    pChar[DeviceType].io_transfer.RxData = (char*)memoryAllocation(IO_BUFFER_SIZE, sizeof(char));
    pChar[DeviceType].io_transfer.TxData = (char*)memoryAllocation(IO_BUFFER_SIZE, sizeof(char));

    /* Check if memory allocation was successful */
    if (!pChar[DeviceType].io_transfer.RxData || !pChar[DeviceType].io_transfer.TxData)
    {
        printk(KERN_ERR "[INIT][ C ] Memory allocation failed\n");

        memoryRelease(pChar[DeviceType].io_transfer.RxData, IO_BUFFER_SIZE, sizeof(char));
        memoryRelease(pChar[DeviceType].io_transfer.TxData, IO_BUFFER_SIZE, sizeof(char));
        return;
    }
    else
    {
        printk(KERN_INFO "[INIT][ C ] %s :: Memory allocated succesfully\n", pChar[DeviceType].name);
    }
}

static void charDeviceConfig(charDeviceType DeviceType)
{
    charDeviceData* pChar = getCharDevice();

    pChar[DeviceType].majorNumber = register_chrdev(0, pChar[DeviceType].name, &pChar[DeviceType].fops);
    if (pChar[DeviceType].majorNumber<0)
    {
        printk(KERN_ALERT "[INIT][ C ] %s :: Failed to register major number\n", pChar[DeviceType].name);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] %s :: Register major number[%d]\n", pChar[DeviceType].name, pChar[DeviceType].majorNumber);
    }

    pChar[DeviceType].deviceClass = class_create(THIS_MODULE, pChar[DeviceType].name);
    if (IS_ERR(pChar[DeviceType].deviceClass))
    {
        unregister_chrdev(pChar[DeviceType].majorNumber, pChar[DeviceType].name);
        printk(KERN_ALERT "[INIT][ C ] %s :: Failed to register device class\n", pChar[DeviceType].name);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] %s :: Register device class\n", pChar[DeviceType].name);
    }

    pChar[DeviceType].nodeDevice = device_create(pChar[DeviceType].deviceClass, NULL, MKDEV(pChar[DeviceType].majorNumber, 0), NULL, pChar[DeviceType].name);
    if (IS_ERR(pChar[DeviceType].nodeDevice))
    {
        class_destroy(pChar[DeviceType].deviceClass);
        unregister_chrdev(pChar[DeviceType].majorNumber, pChar[DeviceType].name);
        printk(KERN_ALERT "[INIT][ C ] Failed to create the device for %s\n", pChar[DeviceType].name);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] %s :: Char Device Succesfully created\n", pChar[DeviceType].name);
    }
}

static void charDeviceConfigDestroy(charDeviceType DeviceType)
{
    charDeviceData* pChar = getCharDevice();

    if(pChar[DeviceType].nodeDevice)
    {
        device_destroy(pChar[DeviceType].deviceClass, MKDEV(pChar[DeviceType].majorNumber, 0));
        pChar[DeviceType].nodeDevice = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Device destroyed\n", pChar[DeviceType].name);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Cannot destroy\n", pChar[DeviceType].name);
    }

    if(pChar[DeviceType].deviceClass)
    {
        class_destroy(pChar[DeviceType].deviceClass);
        pChar[DeviceType].deviceClass = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Class destroyed\n", pChar[DeviceType].name);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Class Cannot destroyed\n", pChar[DeviceType].name);
    }

    if(pChar[DeviceType].majorNumber != 0)
    {
        unregister_chrdev(pChar[DeviceType].majorNumber, pChar[DeviceType].name);
        pChar[DeviceType].majorNumber = 0;
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Char Device Unregistered\n", pChar[DeviceType].name);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Cannot unregister Char Device\n", pChar[DeviceType].name);
    }
    printk(KERN_ALERT "[DESTROY][ C ] %s :: Char Device destruction complete\n", pChar[DeviceType].name);
}

void charDeviceInit(void)
{
    charDeviceType DeviceType;

    printk(KERN_ALERT "[INIT][ C ] Initialize Kernel Mutexes\n");

    for (DeviceType = DEVICE_WATCHDOG; DeviceType < DEVICE_AMOUNT; DeviceType++)
    {
        charDeviceMutexCtrl(DeviceType, MUTEX_CTRL_INIT);
        charDeviceDataInit(DeviceType);
        charDeviceConfig(DeviceType);
    }
}

void charDeviceDestroy(void)
{
    charDeviceType DeviceType;
    charDeviceData* pChar = getCharDevice();

    for (DeviceType = DEVICE_WATCHDOG; DeviceType < DEVICE_AMOUNT; DeviceType++)
    {
        charDeviceConfigDestroy(DeviceType);
        charDeviceMutexCtrl(DeviceType, MUTEX_CTRL_DESTROY);
        memoryRelease(pChar[DeviceType].io_transfer.RxData, IO_BUFFER_SIZE, sizeof(char));
        memoryRelease(pChar[DeviceType].io_transfer.TxData, IO_BUFFER_SIZE, sizeof(char));
    }

    printk(KERN_INFO "[DESTROY][ C ] Kernel Mutexes destroyed\n");
}

