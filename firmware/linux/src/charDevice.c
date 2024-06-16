/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h> 	// Include for copy_to_user function
#include <linux/slab.h> 	// Include for kmalloc/kfree functions
#include <linux/mutex.h>    // Include for mutex opearations
#include <linux/device.h>   // Include for class_create
#include <linux/fs.h>       // Include for file_operations struct

#include "stateMachine.h"
#include "charDevice.h"
#include "spiWork.h"
#include "types.h"

//////////////////////
//                  //
//                  //
//                  //
//   [ x ] Device   //
//                  //
//                  //
//                  //
//////////////////////

#define  DEVICE_COM "KernelInput"
#define  CLASS_COM  "KernelInput"

#define  DEVICE_NET "KernelOutput"
#define  CLASS_NET  "KernelOutput"

static int majorNumber[2];
static struct class*  C_Class[2]  = {NULL,NULL};
static struct device* C_Device[2] = {NULL,NULL};
static int numberOpens[2] = {0,0};

static DEFINE_MUTEX(KernelInput_mutex);
static DEFINE_MUTEX(KernelOutput_mutex);
static DEFINE_MUTEX(wait_mutex);

static int dev_open_kernel_input(struct inode *inodep, struct file *filep);
static ssize_t dev_read_kernel_input(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write_kernel_input(struct file *, const char *, size_t, loff_t *);
static int dev_close_kernel_input(struct inode *inodep, struct file *filep);

static int dev_open_kernel_output(struct inode *inodep, struct file *filep);
static ssize_t dev_read_kernel_output(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write_kernel_output(struct file *, const char *, size_t, loff_t *);
static int dev_close_kernel_output(struct inode *inodep, struct file *filep);

static DataTransfer KernelInputTransfer; 
static DataTransfer KernelOutputTransfer; 

enum deviceTYPE
{
    KERNEL_INPUT,
    KERNEL_OUTPUT
};

static struct file_operations fops[2] =
{
   {
       .open = dev_open_kernel_input,
       .read = dev_read_kernel_input,
       .write = dev_write_kernel_input,
       .release = dev_close_kernel_input,
   },
   {
       .open = dev_open_kernel_output,
       .read = dev_read_kernel_output,
       .write = dev_write_kernel_output,
       .release = dev_close_kernel_output,
   }
};

static void init_charDevice_Data(void)
{
    char *RxData, *TxData;

    /* Allocate memory for RxData */
    RxData = kmalloc(1 + 1, GFP_KERNEL); /* Register + Null terminate */
    if (!RxData) 
    {
        printk(KERN_ALERT "[CTRL][COM] RxData :: Memory allocation failed ");
    }

    /* Allocate memory for TxData */
    TxData = kmalloc(1 + 1, GFP_KERNEL); /* Register + null terminate */
    if (!TxData) 
    {
        printk(KERN_ALERT "[CTRL][COM] TxData :: Memory allocation failed ");
    }

    TxData[0] = 0xBB; /* C Device Preamble */
    TxData[1] = '\0'; /* Null terminator */

    KernelInputTransfer.RxData = RxData;
    KernelInputTransfer.TxData = TxData; /* TODO :: TxData is Dummy 0xBB */
    KernelInputTransfer.length = 2;

    /* Lock and wait until feedback transfer unlock it */
    mutex_lock(&wait_mutex);

    printk(KERN_INFO "[INIT][COM] Initialize charDevice Data :: Lock the mutex\n");
}

/* GET KernelInput TRANSFER */ DataTransfer* get_KernelInputTransfer(void) 
{
    return &KernelInputTransfer;
}

/* SET FEEDBACK TRANSFER */ void set_fpgaFeedbackTransfer(const DataTransfer* transferData)
{
    if (transferData != NULL)
    {
        KernelOutputTransfer = *transferData;
        KernelOutputTransfer.length = 1;
        printk(KERN_INFO "[CTRL][NET] Data set in the KernelOutputTransfer :: KernelOutputTransfer->RxData[0] = 0x%02x \n", KernelOutputTransfer.RxData[0]);
        printk(KERN_INFO "[CTRL][NET] Data set in the KernelOutputTransfer :: KernelOutputTransfer->lenght = %d \n", KernelOutputTransfer.length);
        mutex_unlock(&wait_mutex);
        printk(KERN_INFO "[CTRL][NET] Data Received from FPGA :: Unlock the mutex\n");
    }
    else
    {
        KernelOutputTransfer.length = 1;
        KernelOutputTransfer.RxData[0] = 0xFF;
        mutex_unlock(&wait_mutex);
        printk(KERN_INFO "[CTRL][NET] No FPGA Data Received :: Unlock the mutex\n");
    }
}

/* SET KILL APPLICATION */ void set_killApplication(const DataTransfer* transferData)
{
    if (transferData != NULL)
    {
        KernelOutputTransfer = *transferData;
    }
    else
    {
        printk(KERN_INFO "[CTRL][NET] Kill SIGNAL Received :: Unlock the mutex\n");
        // Handle the error, e.g., log it or assert
    }
    mutex_unlock(&wait_mutex);
}

void charDeviceInit(void)
{
    printk(KERN_ALERT "[INIT][COM] Lock on [C] Device Mutex\n");
    mutex_init(&KernelInput_mutex);

    printk(KERN_ALERT "[INIT][NET] Lock on [C] Device Mutex\n");
    mutex_init(&KernelOutput_mutex);

    printk(KERN_ALERT "[INIT][NET] Lock on Wait Mutex\n");
    mutex_init(&wait_mutex);

    //
    // KernelInput
    //
    majorNumber[KERNEL_INPUT] = register_chrdev(0, DEVICE_COM, &fops[KERNEL_INPUT]);
    if (majorNumber[KERNEL_INPUT]<0)
    {
        printk(KERN_ALERT "[INIT][COM] Failed to register major number: %d\n", majorNumber[KERNEL_INPUT]);
    }
    else
    {
        printk(KERN_ALERT "[INIT][COM] Register major number for char Device: %d\n", majorNumber[KERNEL_INPUT]);
    }

    C_Class[KERNEL_INPUT] = class_create(THIS_MODULE, CLASS_COM);
    if (IS_ERR(C_Class[KERNEL_INPUT]))
    {
        unregister_chrdev(majorNumber[KERNEL_INPUT], DEVICE_COM);
        printk(KERN_ALERT "[INIT][COM] Failed to register device class: %ld\n", PTR_ERR(C_Class[KERNEL_INPUT]));
    }
    else
    {
        printk(KERN_ALERT "[INIT][COM] Register device class\n");
    }
    
    C_Device[KERNEL_INPUT] = device_create(C_Class[KERNEL_INPUT], NULL, MKDEV(majorNumber[KERNEL_INPUT], 0), NULL, DEVICE_COM);
    if (IS_ERR(C_Device[KERNEL_INPUT]))
    {
        class_destroy(C_Class[KERNEL_INPUT]);
        unregister_chrdev(majorNumber[KERNEL_INPUT], DEVICE_COM);
        printk(KERN_ALERT "[INIT][COM] Failed to create the device\n");
    }
    else
    {
        printk(KERN_ALERT "[INIT][COM] Create char Device\n");
    }

    init_charDevice_Data();

    //
    // KernelOutput
    //
    majorNumber[KERNEL_OUTPUT] = register_chrdev(0, DEVICE_NET, &fops[KERNEL_OUTPUT]);
    if (majorNumber[KERNEL_OUTPUT] < 0)
    {
        printk(KERN_ALERT "[INIT][NET] Failed to register major number: %d\n", majorNumber[KERNEL_OUTPUT]);
    }
    else
    {
        printk(KERN_ALERT "[INIT][NET] Register major number for char Device: %d\n", majorNumber[KERNEL_OUTPUT]);
    }

    C_Class[KERNEL_OUTPUT] = class_create(THIS_MODULE, CLASS_NET);
    if (IS_ERR(C_Class[KERNEL_OUTPUT]))
    {
        unregister_chrdev(majorNumber[KERNEL_OUTPUT], DEVICE_NET);
        printk(KERN_ALERT "[INIT][NET] Failed to register device class: %ld\n", PTR_ERR(C_Class[KERNEL_OUTPUT]));
    }
    else
    {
        printk(KERN_ALERT "[INIT][NET] Register device class\n");
    }
    
    C_Device[KERNEL_OUTPUT] = device_create(C_Class[KERNEL_OUTPUT], NULL, MKDEV(majorNumber[KERNEL_OUTPUT], 0), NULL, DEVICE_NET);
    if (IS_ERR(C_Device[KERNEL_OUTPUT]))
    {
        class_destroy(C_Class[KERNEL_OUTPUT]);
        unregister_chrdev(majorNumber[KERNEL_OUTPUT], DEVICE_NET);
        printk(KERN_ALERT "[INIT][NET] Failed to create the device\n");
    }
    else
    {
        printk(KERN_ALERT "[INIT][NET] Create char Device\n");
    }

    /* TODO :: Need data architecture for NET communication */
    // init_charDevice_Data();
}

void charDeviceDestroy(void)
{
    //
    // KernelInput
    //
    if(C_Device[KERNEL_INPUT]) 
    {
        device_destroy(C_Class[KERNEL_INPUT], MKDEV(majorNumber[KERNEL_INPUT], 0));
        C_Device[KERNEL_INPUT] = NULL;
        printk(KERN_INFO "[DESTROY][COM] Device destroyed\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][COM] Canot destroy C_Device[KERNEL_INPUT] :: It is already NULL !\n");
    }

    if(C_Class[KERNEL_INPUT]) 
    {
        class_unregister(C_Class[KERNEL_INPUT]);
        class_destroy(C_Class[KERNEL_INPUT]);
        C_Class[KERNEL_INPUT] = NULL;
        printk(KERN_INFO "[DESTROY][COM] Class destroyed\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][COM] Canot destroy C_Class[KERNEL_INPUT] :: It is already NULL !\n");
    }

    if(majorNumber[KERNEL_INPUT] != 0) 
    {
        unregister_chrdev(majorNumber[KERNEL_INPUT], DEVICE_COM);
        majorNumber[KERNEL_INPUT] = 0;
        printk(KERN_INFO "[DESTROY][COM] Unregistered character device\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][COM] Canot unregister KernelInput Device :: majorNumber[KERNEL_INPUT] is already 0 !\n");
        printk(KERN_INFO "[DESTROY][COM] Device destroyed\n");
    }

    mutex_destroy(&KernelInput_mutex);
    printk(KERN_INFO "[DESTROY][COM] Com Mutex destroyed\n");
    printk(KERN_INFO "[DESTROY][COM] Char device destruction complete\n");

    //
    // KernelOutput
    //
    if(C_Device[KERNEL_OUTPUT]) 
    {
        device_destroy(C_Class[KERNEL_OUTPUT], MKDEV(majorNumber[KERNEL_OUTPUT], 0));
        C_Device[KERNEL_OUTPUT] = NULL;
        printk(KERN_INFO "[DESTROY][NET] Device destroyed\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][NET] Canot destroy C_Device[KERNEL_OUTPUT] :: It is already NULL !\n");
    }

    if(C_Class[KERNEL_OUTPUT]) 
    {
        class_unregister(C_Class[KERNEL_OUTPUT]);
        class_destroy(C_Class[KERNEL_OUTPUT]);
        C_Class[KERNEL_OUTPUT] = NULL;
        printk(KERN_INFO "[DESTROY][NET] Class destroyed\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][NET] Canot destroy C_Class[KERNEL_OUTPUT] :: It is already NULL !\n");
    }

    if(majorNumber[KERNEL_OUTPUT] != 0) 
    {
        unregister_chrdev(majorNumber[KERNEL_OUTPUT], DEVICE_NET);
        majorNumber[KERNEL_OUTPUT] = 0;
        printk(KERN_INFO "[DESTROY][NET] Unregistered character device\n");
    }
    else
    {
        printk(KERN_INFO "[DESTROY][NET] Canot unregister KernelOutput Device :: majorNumber[KERNEL_OUTPUT] is already 0 !\n");
        printk(KERN_INFO "[DESTROY][NET] Device destroyed\n");
    }

    mutex_destroy(&KernelOutput_mutex);
    printk(KERN_INFO "[DESTROY][NET] Net Mutex destroyed\n");

    mutex_destroy(&wait_mutex);
    printk(KERN_INFO "[DESTROY][NET] Wait Mutex destroyed\n");
    printk(KERN_INFO "[DESTROY][NET] Char device destruction complete\n");
}

/**
 * 
 * 
 * KernelInput Interface
 * 
 * 
 */
static int dev_open_kernel_input(struct inode *inodep, struct file *filep)
{
    if(!mutex_trylock(&KernelInput_mutex))
    {
        printk(KERN_ALERT "[CTRL][COM] Device in use by another process");
        return -EBUSY;
    }

    numberOpens[KERNEL_INPUT]++;
    printk(KERN_INFO "[CTRL][COM] Device has been opened %d time(s)\n", numberOpens[KERNEL_INPUT]);

    return 0;
}

static ssize_t dev_read_kernel_input(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;

    /* TODO :: TxData is Dummy 0xBB */
    error_count = copy_to_user(buffer, (const void *)KernelInputTransfer.TxData, KernelInputTransfer.length);

    if (error_count == 0)
    {
        printk(KERN_INFO "[CTRL][COM] Sent %zu characters to user-space\n", KernelInputTransfer.length);
        /* Length == Preamble + Null Terminator */
        return KernelInputTransfer.length; 
    }
    else 
    {
        printk(KERN_INFO "[CTRL][COM] Failed to send %d characters to user-space\n", error_count);
        /* Failed -- return a bad address message (i.e. -14) */
        return -EFAULT;
    }
}

static ssize_t dev_write_kernel_input(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    size_t i;

    /* Copy RxData from user space to kernel space */
    error_count = copy_from_user((void *)KernelInputTransfer.RxData, buffer, len);
    if (error_count != 0) 
    {
        /* Free allocated memory */
        kfree((void *)KernelInputTransfer.RxData);
        /* Copy failed */
        return -EFAULT;
    }

    /* Kill signal from Application */
    if (KernelInputTransfer.RxData[0] == 0xDE && KernelInputTransfer.RxData[1] == 0xAD)
    {
        printk(KERN_INFO "[CTRL][COM] Kill SIGNAL received from Application\n");
        setStateMachine(KILL_APPLICATION);
        return 0;
    }

    /* 20ms delayed :: Read Enable pulse to FIFO */
    if (KernelInputTransfer.RxData[0] == 0x12 && KernelInputTransfer.RxData[1] == 0x34)
    {
        printk(KERN_INFO "[CTRL][COM] Generate FIFO rd_en from Kernel [long pulse] to be cut in FPGA\n");
        setStateMachine(INTERRUPT);
        return 0;
    }

    KernelInputTransfer.RxData[len] = '\0';  /* Null terminate the char array */
    KernelInputTransfer.length = len;

    // Print each character of the RxData array
    for (i = 0; i < KernelInputTransfer.length; i++) 
    {
        printk(KERN_INFO "[CTRL][COM] Received Byte[%zu]: 0x%02x\n", i, (unsigned char)KernelInputTransfer.RxData[i]);
    }

    setStateMachine(SPI);

    return 0;
}

static int dev_close_kernel_input(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[INIT][COM] Unlock [C] Device Mutex\n");
    mutex_unlock(&KernelInput_mutex);
    printk(KERN_INFO "[CTRL][COM] Device successfully closed\n");
    return 0;
}

/**
 * 
 * 
 * KernelOutput Interface
 * 
 * 
 */
static int dev_open_kernel_output(struct inode *inodep, struct file *filep)
{
    if(!mutex_trylock(&KernelOutput_mutex))
    {
        printk(KERN_ALERT "[CTRL][NET] Device in use by another process");
        return -EBUSY;
    }

    numberOpens[KERNEL_OUTPUT]++;
    printk(KERN_INFO "[CTRL][NET] Device has been opened %d time(s)\n", numberOpens[KERNEL_OUTPUT]);

    return 0;
}

static ssize_t dev_read_kernel_output(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;

    printk(KERN_INFO "[CTRL][SPI] Application is waiting for Data\n");
    printk(KERN_INFO "[CTRL][SPI] Kernel is waiting for mutex Unlock\n");
    mutex_lock(&wait_mutex);

    /* TODO :: TxData is Dummy 0xBB */
    error_count = copy_to_user(buffer, (const void *)KernelOutputTransfer.RxData, KernelOutputTransfer.length);

    if (error_count == 0)
    {
        printk(KERN_INFO "[CTRL][COM] Sent %zu characters to user-space\n", KernelOutputTransfer.length);
        /* Length == Preamble + Null Terminator */
        return KernelOutputTransfer.length; 
    }
    else 
    {
        printk(KERN_INFO "[CTRL][COM] Failed to send %d characters to user-space\n", error_count);
        /* Failed -- return a bad address message (i.e. -14) */
        return -EFAULT;
    }
}

static ssize_t dev_write_kernel_output(struct file *filep, const char __user *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    size_t i;

    /* Copy RxData from user space to kernel space */
    error_count = copy_from_user((void *)KernelOutputTransfer.RxData, buffer, len);
    if (error_count != 0) 
    {
        /* Free allocated memory */
        kfree((void *)KernelOutputTransfer.RxData);
        /* Copy failed */
        return -EFAULT;
    }

    KernelOutputTransfer.RxData[len] = '\0';  /* Null terminate the char array */
    KernelOutputTransfer.length = len;

    // Print each character of the RxData array
    for (i = 0; i < KernelOutputTransfer.length; i++) 
    {
        printk(KERN_INFO "[CTRL][COM] Received Byte[%zu]: 0x%02x\n", i, (unsigned char)KernelOutputTransfer.RxData[i]);
    }

    return 0;
}

static int dev_close_kernel_output(struct inode *inodep, struct file *filep)
{
    printk(KERN_ALERT "[INIT][NET] Unlock [C] Device Mutex\n");
    mutex_unlock(&KernelOutput_mutex);
    printk(KERN_INFO "[CTRL][NET] Device successfully closed\n");
    return 0;
}
