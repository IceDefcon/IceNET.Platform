/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "kernelComms.h"
#include "memory.h"

//////////////////////
//                  //
//                  //
//                  //
//   [ C ] Device   //
//                  //
//                  //
//                  //
//////////////////////

static DECLARE_WAIT_QUEUE_HEAD(commanderWaitQueue);

/* COMMANDER */ static int commanderOpen(struct inode *inodep, struct file *filep);
/* COMMANDER */ static ssize_t commanderRead(struct file *, char *, size_t, loff_t *);
/* COMMANDER */ static ssize_t commanderWrite(struct file *, const char *, size_t, loff_t *);
/* COMMANDER */ static int commanderClose(struct inode *inodep, struct file *filep);

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
        .open = commanderOpen,
        .read = commanderRead,
        .write = commanderWrite,
        .release = commanderClose,
    },

    .name = "mainCommander",
    .nameClass = "mainCommanderClass",
    .unlockTimer = 0,
    .wakeUpDevice = false,
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* OPEN */ static int commanderOpen(struct inode *inodep, struct file *filep)
{
    mainCommander.tryLock = mutex_trylock(&mainCommander.deviceMutex);

    if(!mainCommander.tryLock)
    {
        printk(KERN_ALERT "[CTRL][ C ] Device in use by another process");
        return -EBUSY;
    }

    mainCommander.openCount++;
    printk(KERN_INFO "[CTRL][ C ] DEVICE_COMMANDER has been opened %d time(s)\n", mainCommander.openCount);

    return 0;
}

/* READ */ static ssize_t commanderRead(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int ret = 0;
    int error_count = 0;
    size_t i;

    /* TODO :: Unfreeze if not kicked */
    wait_event_interruptible(commanderWaitQueue, mainCommander.wakeUpDevice);
    mainCommander.wakeUpDevice = false;

    error_count = copy_to_user(buffer, (const void *)mainCommander.io_transfer.TxData, mainCommander.transferSize);

    if (error_count == 0)
    {
#if 0
        printk(KERN_INFO "[CTRL][ C ] Sent %zu characters to user-space\n", mainCommander.transferSize);
#endif
        ret = mainCommander.transferSize;
    }
    else
    {
        printk(KERN_INFO "[CTRL][ C ] Failed to send %d characters to user-space\n", error_count);
        ret = -EFAULT; /* Failed -- return a bad address message (i.e. -14) */
    }

    for (i = 0; i < mainCommander.transferSize; ++i)
    {
        mainCommander.io_transfer.TxData[i] = 0x00;
    }

    return ret;
}

/* WRITE */ static ssize_t commanderWrite(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    int i = 0;
    int ret = 0;
    int error_count = 0;

    printk(KERN_INFO "[CTRL][ C ] Enter commanderWrite \n");
    /* Clear the buffer before we receive data from User Space */
    mainCommander.io_transfer.RxData[0] = 0x00;
    mainCommander.io_transfer.RxData[1] = 0x00;
    mainCommander.io_transfer.RxData[2] = 0x00;
    mainCommander.io_transfer.RxData[3] = 0x00;
    mainCommander.io_transfer.RxData[4] = 0x00;
    mainCommander.io_transfer.RxData[5] = 0x00;
    mainCommander.io_transfer.RxData[6] = 0x00;
    mainCommander.io_transfer.RxData[7] = 0x00;

    /* Copy RxData from user space to kernel space */
    error_count = copy_from_user((void *)mainCommander.io_transfer.RxData, buffer, len);
    if (error_count != 0)
    {
        /* Copy failed */
        ret = -EFAULT;
    }
    else if (mainCommander.io_transfer.RxData[0] == 0x04 && mainCommander.io_transfer.RxData[1] == 0xA1)
    {
        /* Reconfigure DMA Engine */
        // printk(KERN_INFO "[CTRL][ C ] Reconfigure DMA Engine into normal mode\n");
        // setStateMachine(SM_DMA_NORMAL);
    }
    else if (mainCommander.io_transfer.RxData[0] == 0x5E && mainCommander.io_transfer.RxData[1] == 0x50)
    {
        /* Reconfigure DMA Engine */
        // printk(KERN_INFO "[CTRL][ C ] Reconfigure DMA Engine into sensor mode\n");
        // setStateMachine(SM_DMA_SENSOR);
    }
    else if (mainCommander.io_transfer.RxData[0] == 0x51 && mainCommander.io_transfer.RxData[1] == 0x6E)
    {
        /* Reconfigure DMA Engine */
        // printk(KERN_INFO "[CTRL][ C ] Reconfigure DMA Engine into single byte feedback mode\n");
        // setStateMachine(SM_DMA_SINGLE);
    }
    else if (mainCommander.io_transfer.RxData[0] == 0xC5 && mainCommander.io_transfer.RxData[1] == 0x70)
    {
        /* Reconfigure DMA Engine */
        // customDmaSize = mainCommander.io_transfer.RxData[2];
        // printk(KERN_INFO "[CTRL][ C ] Reconfigure DMA Engine into custom %d Byte feedback mode\n", customDmaSize);
        // setStateMachine(SM_DMA_CUSTOM);
    }
    else if (mainCommander.io_transfer.RxData[0] == 0xC0 && mainCommander.io_transfer.RxData[1] == 0xF1)
    {
        /* Activate DMA Engine */
        // printk(KERN_INFO "[CTRL][ C ] Activate DMA transfer to send IMU's config to FPGA\n");
        // setStateMachine(SM_RAMDISK_CONFIG);
    }
    else if (mainCommander.io_transfer.RxData[0] == 0xC1 && mainCommander.io_transfer.RxData[1] == 0xEA)
    {
        // printk(KERN_INFO "[CTRL][ C ] [0] Clear DMA variables used for verification of IMU's config\n");
        // setStateMachine(SM_RAMDISK_CLEAR);
    }
    else if (mainCommander.io_transfer.RxData[0] == 0xDE && mainCommander.io_transfer.RxData[1] == 0xBE)
    {
        // printk(KERN_INFO "[CTRL][ C ] Enable SPI/DMA Debug\n");
        // ctrlDebug(DEBUG_SPI, true);
    }
    else if (mainCommander.io_transfer.RxData[0] == 0xDE && mainCommander.io_transfer.RxData[1] == 0xBD)
    {
        // printk(KERN_INFO "[CTRL][ C ] Disable SPI/DMA Debug\n");
        // ctrlDebug(DEBUG_SPI, false);
    }
    else if (mainCommander.io_transfer.RxData[0] == 0x4E && mainCommander.io_transfer.RxData[1] == 0x5E)
    {
        // printk(KERN_INFO "[CTRL][ C ] Reset everything in FPGA :: Global Discharge\n");
        // setStateMachine(SM_FPGA_RESET);
    }
    else
    {
        printk(KERN_INFO "[CTRL][ C ] Data Received: \n");
        for (i = 0; i < len; i++)
        {
            printk(KERN_INFO "[CTRL][ C ] Byte[%d] %x\n", i, mainCommander.io_transfer.RxData[i]);
        }
        // setStateMachine(SM_PRIMARY_SPI);
    }
    return ret;
}

/* CLOSE */ static int commanderClose(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[CTRL][ C ] DEVICE_COMMANDER Unlock Mutex\n");
    mutex_unlock(&mainCommander.deviceMutex);
    printk(KERN_INFO "[CTRL][ C ] DEVICE_COMMANDER Close\n");
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
        printk(KERN_ALERT "[INIT][ C ] %s :: Succesfully created\n", mainCommander.name);
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

