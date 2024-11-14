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
//   [ C ] Control  //
//                  //
//                  //
//                  //
//////////////////////

static void charDeviceDataInit(void)
{
    char *inputRxData, *inputTxData;
    char *outputRxData, *outputTxData;
    char *watchdogRxData, *watchdogTxData;

    /* Allocate memory */
    inputRxData = (char *)kmalloc(IO_BUFFER_SIZE * sizeof(char), GFP_KERNEL);
    inputTxData = (char *)kmalloc(IO_BUFFER_SIZE * sizeof(char), GFP_KERNEL);
    outputRxData = (char *)kmalloc(IO_BUFFER_SIZE * sizeof(char), GFP_KERNEL);
    outputTxData = (char *)kmalloc(IO_BUFFER_SIZE * sizeof(char), GFP_KERNEL);
    watchdogRxData = (char *)kmalloc(IO_BUFFER_SIZE * sizeof(char), GFP_KERNEL);
    watchdogTxData = (char *)kmalloc(IO_BUFFER_SIZE * sizeof(char), GFP_KERNEL);

    /* Check if memory allocation was successful */
    if (!inputRxData || !inputTxData || !outputRxData || !outputTxData)
    {
        printk(KERN_ERR "[INIT][ C ] Memory allocation failed\n");
        kfree(inputRxData);
        kfree(inputTxData);
        kfree(outputRxData);
        kfree(outputTxData);
        kfree(watchdogRxData);
        kfree(watchdogTxData);
        return;
    }
    else
    {
        printk(KERN_INFO "[INIT][ C ] Memory allocated succesfully for all char devices\n");
    }

    getCharDevice()[DEVICE_INPUT].io_transfer.RxData = inputRxData;
    getCharDevice()[DEVICE_INPUT].io_transfer.TxData = inputTxData;
    getCharDevice()[DEVICE_INPUT].io_transfer.length = IO_BUFFER_SIZE;

    getCharDevice()[DEVICE_OUTPUT].io_transfer.RxData = outputRxData;
    getCharDevice()[DEVICE_OUTPUT].io_transfer.TxData = outputTxData;
    getCharDevice()[DEVICE_OUTPUT].io_transfer.length = IO_BUFFER_SIZE;

    getCharDevice()[DEVICE_WATCHDOG].io_transfer.RxData = watchdogRxData;
    getCharDevice()[DEVICE_WATCHDOG].io_transfer.TxData = watchdogTxData;
    getCharDevice()[DEVICE_WATCHDOG].io_transfer.length = IO_BUFFER_SIZE;
}

void charDeviceInit(void)
{
    printk(KERN_ALERT "[INIT][ C ] Initialize Kernel Mutexes\n");
    charDeviceMutexCtrl(DEVICE_INPUT, MUTEX_CTRL_INIT);
    charDeviceMutexCtrl(DEVICE_OUTPUT, MUTEX_CTRL_INIT);
    charDeviceMutexCtrl(DEVICE_WATCHDOG, MUTEX_CTRL_INIT);

    charDeviceDataInit();

    //
    // KernelInput
    //
    getCharDevice()[DEVICE_INPUT].majorNumber = register_chrdev(0, INPUT_DEVICE, &getCharDevice()[DEVICE_INPUT].fops);
    if (getCharDevice()[DEVICE_INPUT].majorNumber<0)
    {
        printk(KERN_ALERT "[INIT][ C ] Failed to register major number for %s :: %d\n", INPUT_DEVICE, getCharDevice()[DEVICE_INPUT].majorNumber);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register major number for %s :: %d\n", INPUT_DEVICE, getCharDevice()[DEVICE_INPUT].majorNumber);
    }

    getCharDevice()[DEVICE_INPUT].deviceClass = class_create(THIS_MODULE, INPUT_CLASS);
    if (IS_ERR(getCharDevice()[DEVICE_INPUT].deviceClass))
    {
        unregister_chrdev(getCharDevice()[DEVICE_INPUT].majorNumber, INPUT_DEVICE);
        printk(KERN_ALERT "[INIT][ C ] Failed to register device class for %s :: %ld\n", INPUT_DEVICE, PTR_ERR(getCharDevice()[DEVICE_INPUT].deviceClass));
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register device class for %s\n", INPUT_DEVICE);
    }

    getCharDevice()[DEVICE_INPUT].nodeDevice = device_create(getCharDevice()[DEVICE_INPUT].deviceClass, NULL, MKDEV(getCharDevice()[DEVICE_INPUT].majorNumber, 0), NULL, INPUT_DEVICE);
    if (IS_ERR(getCharDevice()[DEVICE_INPUT].nodeDevice))
    {
        class_destroy(getCharDevice()[DEVICE_INPUT].deviceClass);
        unregister_chrdev(getCharDevice()[DEVICE_INPUT].majorNumber, INPUT_DEVICE);
        printk(KERN_ALERT "[INIT][ C ] Failed to create the device for %s\n", INPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Succesfully created char Device for %s\n", INPUT_DEVICE);
    }
    //
    // KernelOutput
    //
    getCharDevice()[DEVICE_OUTPUT].majorNumber = register_chrdev(0, OUTPUT_DEVICE, &getCharDevice()[DEVICE_OUTPUT].fops);
    if (getCharDevice()[DEVICE_OUTPUT].majorNumber < 0)
    {
        printk(KERN_ALERT "[INIT][ C ] Failed to register major number for %s :: %d\n", OUTPUT_DEVICE, getCharDevice()[DEVICE_OUTPUT].majorNumber);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register major number for %s :: %d\n", OUTPUT_DEVICE, getCharDevice()[DEVICE_OUTPUT].majorNumber);
    }

    getCharDevice()[DEVICE_OUTPUT].deviceClass = class_create(THIS_MODULE, OUTPUT_CLASS);
    if (IS_ERR(getCharDevice()[DEVICE_OUTPUT].deviceClass))
    {
        unregister_chrdev(getCharDevice()[DEVICE_OUTPUT].majorNumber, OUTPUT_DEVICE);
        printk(KERN_ALERT "[INIT][ C ] Failed to register device class for %s :: %ld\n", OUTPUT_DEVICE, PTR_ERR(getCharDevice()[DEVICE_OUTPUT].deviceClass));
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register device class for %s\n", OUTPUT_DEVICE);
    }

    getCharDevice()[DEVICE_OUTPUT].nodeDevice = device_create(getCharDevice()[DEVICE_OUTPUT].deviceClass, NULL, MKDEV(getCharDevice()[DEVICE_OUTPUT].majorNumber, 0), NULL, OUTPUT_DEVICE);
    if (IS_ERR(getCharDevice()[DEVICE_OUTPUT].nodeDevice))
    {
        class_destroy(getCharDevice()[DEVICE_OUTPUT].deviceClass);
        unregister_chrdev(getCharDevice()[DEVICE_OUTPUT].majorNumber, OUTPUT_DEVICE);
        printk(KERN_ALERT "[INIT][ C ] Failed to create the device for %s\n", OUTPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Succesfully created char Device for %s\n", OUTPUT_DEVICE);
    }
    //
    // Watchdog
    //
    getCharDevice()[DEVICE_WATCHDOG].majorNumber = register_chrdev(0, WATCHDOG_DEVICE, &getCharDevice()[DEVICE_WATCHDOG].fops);
    if (getCharDevice()[DEVICE_WATCHDOG].majorNumber < 0)
    {
        printk(KERN_ALERT "[INIT][ C ] Failed to register major number for %s :: %d\n", WATCHDOG_DEVICE, getCharDevice()[DEVICE_WATCHDOG].majorNumber);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register major number for %s :: %d\n", WATCHDOG_DEVICE, getCharDevice()[DEVICE_WATCHDOG].majorNumber);
    }

    getCharDevice()[DEVICE_WATCHDOG].deviceClass = class_create(THIS_MODULE, WATCHDOG_CLASS);
    if (IS_ERR(getCharDevice()[DEVICE_WATCHDOG].deviceClass))
    {
        unregister_chrdev(getCharDevice()[DEVICE_WATCHDOG].majorNumber, WATCHDOG_DEVICE);
        printk(KERN_ALERT "[INIT][ C ] Failed to register device class for %s :: %ld\n", WATCHDOG_DEVICE, PTR_ERR(getCharDevice()[DEVICE_WATCHDOG].deviceClass));
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Register device class for %s\n", WATCHDOG_DEVICE);
    }

    getCharDevice()[DEVICE_WATCHDOG].nodeDevice = device_create(getCharDevice()[DEVICE_WATCHDOG].deviceClass, NULL, MKDEV(getCharDevice()[DEVICE_WATCHDOG].majorNumber, 0), NULL, WATCHDOG_DEVICE);
    if (IS_ERR(getCharDevice()[DEVICE_WATCHDOG].nodeDevice))
    {
        class_destroy(getCharDevice()[DEVICE_WATCHDOG].deviceClass);
        unregister_chrdev(getCharDevice()[DEVICE_WATCHDOG].majorNumber, WATCHDOG_DEVICE);
        printk(KERN_ALERT "[INIT][ C ] Failed to create the device for %s\n", WATCHDOG_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] Succesfully created char Device for %s\n", WATCHDOG_DEVICE);
    }
}

void charDeviceDestroy(void)
{
    //
    // KernelInput
    //
    if(getCharDevice()[DEVICE_INPUT].nodeDevice)
    {
        device_destroy(getCharDevice()[DEVICE_INPUT].deviceClass, MKDEV(getCharDevice()[DEVICE_INPUT].majorNumber, 0));
        getCharDevice()[DEVICE_INPUT].nodeDevice = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s Device destroyed\n", INPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot destroy %s\n", INPUT_DEVICE);
    }

    if(getCharDevice()[DEVICE_INPUT].deviceClass)
    {
        class_unregister(getCharDevice()[DEVICE_INPUT].deviceClass);
        class_destroy(getCharDevice()[DEVICE_INPUT].deviceClass);
        getCharDevice()[DEVICE_INPUT].deviceClass = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s Class destroyed\n", INPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot destroy %s Class\n", INPUT_DEVICE);
    }

    if(getCharDevice()[DEVICE_INPUT].majorNumber != 0)
    {
        unregister_chrdev(getCharDevice()[DEVICE_INPUT].majorNumber, INPUT_DEVICE);
        getCharDevice()[DEVICE_INPUT].majorNumber = 0;
        printk(KERN_ALERT "[DESTROY][ C ] Unregistered %s device\n", INPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot unregister %s Device\n", INPUT_DEVICE);
    }
    printk(KERN_ALERT "[DESTROY][ C ] %s device destruction complete\n", INPUT_DEVICE);

    //
    // KernelOutput
    //
    if(getCharDevice()[DEVICE_OUTPUT].nodeDevice)
    {
        device_destroy(getCharDevice()[DEVICE_OUTPUT].deviceClass, MKDEV(getCharDevice()[DEVICE_OUTPUT].majorNumber, 0));
        getCharDevice()[DEVICE_OUTPUT].nodeDevice = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s Device destroyed\n", OUTPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot destroy %s\n", OUTPUT_DEVICE);
    }

    if(getCharDevice()[DEVICE_OUTPUT].deviceClass)
    {
        class_unregister(getCharDevice()[DEVICE_OUTPUT].deviceClass);
        class_destroy(getCharDevice()[DEVICE_OUTPUT].deviceClass);
        getCharDevice()[DEVICE_OUTPUT].deviceClass = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s Class destroyed\n", OUTPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot destroy %s Class\n", OUTPUT_DEVICE);
    }

    if(getCharDevice()[DEVICE_OUTPUT].majorNumber != 0)
    {
        unregister_chrdev(getCharDevice()[DEVICE_OUTPUT].majorNumber, OUTPUT_DEVICE);
        getCharDevice()[DEVICE_OUTPUT].majorNumber = 0;
        printk(KERN_ALERT "[DESTROY][ C ] Unregistered %s device\n", OUTPUT_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot unregister %s Device\n", OUTPUT_DEVICE);

    }
    printk(KERN_ALERT "[DESTROY][ C ] %s device destruction complete\n", OUTPUT_DEVICE);

    //
    // Watchdog
    //
    if(getCharDevice()[DEVICE_WATCHDOG].nodeDevice)
    {
        device_destroy(getCharDevice()[DEVICE_WATCHDOG].deviceClass, MKDEV(getCharDevice()[DEVICE_WATCHDOG].majorNumber, 0));
        getCharDevice()[DEVICE_WATCHDOG].nodeDevice = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s Device destroyed\n", WATCHDOG_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot destroy %s\n", WATCHDOG_DEVICE);
    }

    if(getCharDevice()[DEVICE_WATCHDOG].deviceClass)
    {
        class_unregister(getCharDevice()[DEVICE_WATCHDOG].deviceClass);
        class_destroy(getCharDevice()[DEVICE_WATCHDOG].deviceClass);
        getCharDevice()[DEVICE_WATCHDOG].deviceClass = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s Class destroyed\n", WATCHDOG_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot destroy %s Class\n", WATCHDOG_DEVICE);
    }

    if(getCharDevice()[DEVICE_WATCHDOG].majorNumber != 0)
    {
        unregister_chrdev(getCharDevice()[DEVICE_WATCHDOG].majorNumber, WATCHDOG_DEVICE);
        getCharDevice()[DEVICE_WATCHDOG].majorNumber = 0;
        printk(KERN_ALERT "[DESTROY][ C ] Unregistered %s device\n", WATCHDOG_DEVICE);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] Cannot unregister %s Device\n", WATCHDOG_DEVICE);
    }
    printk(KERN_ALERT "[DESTROY][ C ] %s device destruction complete\n", WATCHDOG_DEVICE);

    charDeviceMutexCtrl(DEVICE_INPUT, MUTEX_CTRL_DESTROY);
    charDeviceMutexCtrl(DEVICE_OUTPUT, MUTEX_CTRL_DESTROY);
    charDeviceMutexCtrl(DEVICE_WATCHDOG, MUTEX_CTRL_DESTROY);

    printk(KERN_INFO "[DESTROY][ C ] Kernel Mutexes destroyed\n");
}

