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
#include <linux/slab.h>     // Include for kmalloc/kfree functions
#include <linux/mutex.h>    // Include for mutex opearations
#include <linux/device.h>   // Include for class_create

#include "stateMachine.h"
#include "charDevice.h"
#include "spiWork.h"
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
    char *RxData, *TxData;
    charDeviceData* pChar = getCharDevice();

    /* Allocate memory */
    RxData = (char *)kmalloc(IO_BUFFER_SIZE * sizeof(char), GFP_KERNEL);
    TxData = (char *)kmalloc(IO_BUFFER_SIZE * sizeof(char), GFP_KERNEL);

    /* Check if memory allocation was successful */
    if (!RxData || !TxData)
    {
        printk(KERN_ERR "[INIT][ C ] Memory allocation failed\n");
        kfree(RxData);
        kfree(TxData);
        return;
    }
    else
    {
        printk(KERN_INFO "[INIT][ C ] %s :: Memory allocated succesfully\n", pChar[DeviceType].name);
    }

    pChar[DeviceType].io_transfer.RxData = RxData;
    pChar[DeviceType].io_transfer.TxData = TxData;
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

    for (DeviceType = DEVICE_WATCHDOG; DeviceType < DEVICE_AMOUNT; DeviceType++)
    {
        charDeviceConfigDestroy(DeviceType);
        charDeviceMutexCtrl(DeviceType, MUTEX_CTRL_DESTROY);
    }

    printk(KERN_INFO "[DESTROY][ C ] Kernel Mutexes destroyed\n");
}

