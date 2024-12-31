/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2024
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
        printk(KERN_INFO "[INIT][ C ] Memory allocated succesfully for all char devices\n");
    }

    getCharDevice()[DeviceType].io_transfer.RxData = RxData;
    getCharDevice()[DeviceType].io_transfer.TxData = TxData;
    getCharDevice()[DeviceType].io_transfer.length = IO_BUFFER_SIZE;
}

static void charDeviceConfig(charDeviceType DeviceType)
{
    getCharDevice()[DeviceType].majorNumber = register_chrdev(0, getCharDevice()[DeviceType].name, &getCharDevice()[DeviceType].fops);
    if (getCharDevice()[DeviceType].majorNumber<0)
    {
        printk(KERN_ALERT "[INIT][ C ] Failed to register major number for %s :: %d\n", getCharDevice()[DeviceType].name, getCharDevice()[DeviceType].majorNumber);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register major number for %s :: %d\n", getCharDevice()[DeviceType].name, getCharDevice()[DeviceType].majorNumber);
    }

    getCharDevice()[DeviceType].deviceClass = class_create(THIS_MODULE, getCharDevice()[DeviceType].name);
    if (IS_ERR(getCharDevice()[DeviceType].deviceClass))
    {
        unregister_chrdev(getCharDevice()[DeviceType].majorNumber, getCharDevice()[DeviceType].name);
        printk(KERN_ALERT "[INIT][ C ] Failed to register device class for %s :: %ld\n", getCharDevice()[DeviceType].name, PTR_ERR(getCharDevice()[DeviceType].deviceClass));
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register device class for %s\n", getCharDevice()[DeviceType].name);
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
        printk(KERN_ALERT "[INIT][ C ] Succesfully created char Device for %s\n", getCharDevice()[DeviceType].name);
    }
}

static void charDeviceConfigDestroy(charDeviceType DeviceType)
{
    if(getCharDevice()[DeviceType].nodeDevice)
    {
        device_destroy(getCharDevice()[DeviceType].deviceClass, MKDEV(getCharDevice()[DeviceType].majorNumber, 0));
        getCharDevice()[DeviceType].nodeDevice = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s Device destroyed\n", getCharDevice()[DeviceType].name);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot destroy %s\n", getCharDevice()[DeviceType].name);
    }

    if(getCharDevice()[DeviceType].deviceClass)
    {
        class_destroy(getCharDevice()[DeviceType].deviceClass);
        getCharDevice()[DeviceType].deviceClass = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s Class destroyed\n", getCharDevice()[DeviceType].name);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot destroy %s Class\n", getCharDevice()[DeviceType].name);
    }

    if(getCharDevice()[DeviceType].majorNumber != 0)
    {
        unregister_chrdev(getCharDevice()[DeviceType].majorNumber, getCharDevice()[DeviceType].name);
        getCharDevice()[DeviceType].majorNumber = 0;
        printk(KERN_ALERT "[DESTROY][ C ] Unregistered %s device\n", getCharDevice()[DeviceType].name);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot unregister %s Device\n", getCharDevice()[DeviceType].name);
    }
    printk(KERN_ALERT "[DESTROY][ C ] %s device destruction complete\n", getCharDevice()[DeviceType].name);
}

void charDeviceInit(void)
{
    charDeviceType DeviceType;

    printk(KERN_ALERT "[INIT][ C ] Initialize Kernel Mutexes\n");

    for (DeviceType = DEVICE_INPUT; DeviceType < DEVICE_AMOUNT; DeviceType++)
    {
        charDeviceMutexCtrl(DeviceType, MUTEX_CTRL_INIT);
        charDeviceDataInit(DeviceType);
        charDeviceConfig(DeviceType);
    }
}

void charDeviceDestroy(void)
{
    charDeviceType DeviceType;

    for (DeviceType = DEVICE_INPUT; DeviceType < DEVICE_AMOUNT; DeviceType++)
    {
        charDeviceConfigDestroy(DeviceType);
        charDeviceMutexCtrl(DeviceType, MUTEX_CTRL_DESTROY);
    }

    printk(KERN_INFO "[DESTROY][ C ] Kernel Mutexes destroyed\n");
}

