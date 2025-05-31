/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "kernelComms.h"
#include "memory.h"

static charDeviceData mainCommander =
{
    .majorNumber = 0,
    .deviceClass = NULL,
    .nodeDevice = NULL,
    .openCount = 0,
    .deviceMutex = __MUTEX_INITIALIZER(mainCommander.deviceMutex),
    .tryLock = 0,
    .transferSize = 6,

    .io_transfer =
    {
        .RxData = NULL,
        .TxData = NULL,
    },

    .fops =
    {
        .open = NULL,//commanderOpen,
        .read = NULL,//commanderRead,
        .write = NULL,//commanderWrite,
        .release = NULL,//commanderClose,
    },

    .name = "mainCommander",
    .nameClass = "mainCommanderClass",
    .unlockTimer = 0,
    .wakeUpDevice = false,
};

static void allocateMainCommanderData(void)
{
    /* Allocate memory */
    mainCommander.io_transfer.RxData = (char*)memoryAllocation(MAIN_COMMANDER_BUFFER_SIZE, sizeof(char));
    mainCommander.io_transfer.TxData = (char*)memoryAllocation(MAIN_COMMANDER_BUFFER_SIZE, sizeof(char));

    /* Check if memory allocation was successful */
    if (!mainCommander.io_transfer.RxData || !mainCommander.io_transfer.TxData)
    {
        printk(KERN_ERR "[INIT][ C ] Memory allocation failed\n");

        memoryRelease(mainCommander.io_transfer.RxData, MAIN_COMMANDER_BUFFER_SIZE, sizeof(char));
        memoryRelease(mainCommander.io_transfer.TxData, MAIN_COMMANDER_BUFFER_SIZE, sizeof(char));
        return;
    }
    else
    {
        printk(KERN_INFO "[INIT][ C ] %s :: Memory allocated succesfully\n", mainCommander.name);
    }
}

static void registerMainComander(void)
{
    mainCommander.majorNumber = register_chrdev(0, mainCommander.name, &mainCommander.fops);
    if (mainCommander.majorNumber<0)
    {
        printk(KERN_ALERT "[INIT][ C ] %s :: Failed to register major number\n", mainCommander.name);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] %s :: Register major number[%d]\n", mainCommander.name, mainCommander.majorNumber);
    }

    /* Replaced in 6.11 Kernel */
    mainCommander.deviceClass = class_create(mainCommander.name);
    if (IS_ERR(mainCommander.deviceClass))
    {
        unregister_chrdev(mainCommander.majorNumber, mainCommander.name);
        printk(KERN_ALERT "[INIT][ C ] %s :: Failed to register mainCommander class\n", mainCommander.name);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] %s :: Register mainCommander class\n", mainCommander.name);
    }

    mainCommander.nodeDevice = device_create(mainCommander.deviceClass, NULL, MKDEV(mainCommander.majorNumber, 0), NULL, mainCommander.name);
    if (IS_ERR(mainCommander.nodeDevice))
    {
        class_destroy(mainCommander.deviceClass);
        unregister_chrdev(mainCommander.majorNumber, mainCommander.name);
        printk(KERN_ALERT "[INIT][ C ] Failed to create the mainCommander for %s\n", mainCommander.name);
    }
    else
    {
        printk(KERN_ALERT "[INIT][ C ] %s :: Char mainCommander Succesfully created\n", mainCommander.name);
    }
}

static void unregisterMainCommander(void)
{

    if(mainCommander.nodeDevice)
    {
        device_destroy(mainCommander.deviceClass, MKDEV(mainCommander.majorNumber, 0));
        mainCommander.nodeDevice = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s :: mainCommander destroyed\n", mainCommander.name);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Cannot destroy\n", mainCommander.name);
    }

    if(mainCommander.deviceClass)
    {
        class_destroy(mainCommander.deviceClass);
        mainCommander.deviceClass = NULL;
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Class destroyed\n", mainCommander.name);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Class Cannot destroyed\n", mainCommander.name);
    }

    if(mainCommander.majorNumber != 0)
    {
        unregister_chrdev(mainCommander.majorNumber, mainCommander.name);
        mainCommander.majorNumber = 0;
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Char mainCommander Unregistered\n", mainCommander.name);
    }
    else
    {
        printk(KERN_ALERT "[DESTROY][ C ] %s :: Cannot unregister Char mainCommander\n", mainCommander.name);
    }
    printk(KERN_ALERT "[DESTROY][ C ] %s :: Char mainCommander destruction complete\n", mainCommander.name);
}

void mainCommanderInit(void)
{
    printk(KERN_ALERT "[INIT][ C ] Initialize Main Commander\n");

    mutex_init(&mainCommander.deviceMutex);
    allocateMainCommanderData();
    registerMainComander();
}

void mainCommanderDestroy(void)
{
    unregisterMainCommander();

    mainCommander.tryLock = mutex_trylock(&mainCommander.deviceMutex);
    if (mainCommander.tryLock  == 0)
    {
        mutex_unlock(&mainCommander.deviceMutex);
    }
    mutex_destroy(&mainCommander.deviceMutex);

    memoryRelease(mainCommander.io_transfer.RxData, MAIN_COMMANDER_BUFFER_SIZE, sizeof(char));
    memoryRelease(mainCommander.io_transfer.TxData, MAIN_COMMANDER_BUFFER_SIZE, sizeof(char));

    printk(KERN_INFO "[DESTROY][ C ] Main Commander destroyed\n");
}

