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
        printk(KERN_INFO "[INIT][ C ] %s :: Memory allocated succesfully\n", getCharDevice()[DeviceType].name);
    }

    getCharDevice()[DeviceType].io_transfer.RxData = RxData;
    getCharDevice()[DeviceType].io_transfer.TxData = TxData;
    getCharDevice()[DeviceType].transferSize = IO_BUFFER_SIZE;
}

static void charDeviceConfig(charDeviceType DeviceType)
{
    getCharDevice()[DeviceType].majorNumber = register_chrdev(0, getCharDevice()[DeviceType].name, &getCharDevice()[DeviceType].fops);
    if (getCharDevice()[DeviceType].majorNumber<0)
    {
        printk(KERN_ALERT "[INIT][ C ] %s :: Failed to register major number\n", getCharDevice()[DeviceType].name);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] %s :: Register major number[%d]\n", getCharDevice()[DeviceType].name, getCharDevice()[DeviceType].majorNumber);
    }

    getCharDevice()[DeviceType].deviceClass = class_create(THIS_MODULE, getCharDevice()[DeviceType].name);
    if (IS_ERR(getCharDevice()[DeviceType].deviceClass))
    {
        unregister_chrdev(getCharDevice()[DeviceType].majorNumber, getCharDevice()[DeviceType].name);
        printk(KERN_ALERT "[INIT][ C ] %s :: Failed to register device class\n", getCharDevice()[DeviceType].name);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] %s :: Register device class\n", getCharDevice()[DeviceType].name);
    }

    getCharDevice()[DeviceType].nodeDevice = device_create(getCharDevice()[DeviceType].deviceClass, NULL, MKDEV(getCharDevice()[DeviceType].majorNumber, 0), NULL, getCharDevice()[DeviceType].name);
    if (IS_ERR(getCharDevice()[DeviceType].nodeDevice))
    {
        class_destroy(getCharDevice()[DeviceType].deviceClass);
        unregister_chrdev(getCharDevice()[DeviceType].majorNumber, getCharDevice()[DeviceType].name);
        printk(KERN_ALERT "[INIT][ C ] Failed to create the device for %s\n", getCharDevice()[DeviceType].name);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] %s :: Char Device Succesfully created\n", getCharDevice()[DeviceType].name);
    }
}

static void charDeviceConfigDestroy(charDeviceType DeviceType)
{
    if(getCharDevice()[DeviceType].nodeDevice)
    {
        device_destroy(getCharDevice()[DeviceType].deviceClass, MKDEV(getCharDevice()[DeviceType].majorNumber, 0));
        getCharDevice()[DeviceType].nodeDevice = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Device destroyed\n", getCharDevice()[DeviceType].name);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Cannot destroy\n", getCharDevice()[DeviceType].name);
    }

    if(getCharDevice()[DeviceType].deviceClass)
    {
        class_destroy(getCharDevice()[DeviceType].deviceClass);
        getCharDevice()[DeviceType].deviceClass = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Class destroyed\n", getCharDevice()[DeviceType].name);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Class Cannot destroyed\n", getCharDevice()[DeviceType].name);
    }

    if(getCharDevice()[DeviceType].majorNumber != 0)
    {
        unregister_chrdev(getCharDevice()[DeviceType].majorNumber, getCharDevice()[DeviceType].name);
        getCharDevice()[DeviceType].majorNumber = 0;
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Char Device Unregistered\n", getCharDevice()[DeviceType].name);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Cannot unregister Char Device\n", getCharDevice()[DeviceType].name);
    }
    printk(KERN_ALERT "[DESTROY][ C ] %s :: Char Device destruction complete\n", getCharDevice()[DeviceType].name);
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

