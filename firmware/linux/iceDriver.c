//
// Author: Ice.Marek
// IceNET Technology 2023
//

#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/spi/spi.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h> // For workqueue-related functions and macros
#include <linux/slab.h>      // For memory allocation functions like kmalloc

MODULE_VERSION("2.0");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marek Ice");
MODULE_DESCRIPTION("FPGA Comms Driver");

//////////////////////
//                  //
//                  //
//                  //
//    [C] Device    //
//                  //
//                  //
//                  //
//////////////////////

#define  DEVICE_NAME "iceCOM"
#define  CLASS_NAME  "iceCOM"

static int    majorNumber;
static char   message[256] = {0};
static unsigned long  size_of_message;
static int    numberOpens = 0;
static struct class*  C_Class  = NULL;
static struct device* C_Device = NULL;

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static DEFINE_MUTEX(com_mutex);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

static int dev_open(struct inode *inodep, struct file *filep)
{
    if(!mutex_trylock(&com_mutex))
    {
        printk(KERN_ALERT "[FPGA][ C ] Device in use by another process");
        return -EBUSY;
    }

    numberOpens++;
    printk(KERN_INFO "[FPGA][ C ] Device has been opened %d time(s)\n", numberOpens);
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    //
    // Copy to user space :: *to, *from, size :: returns 0 on success
    //
    error_count = copy_to_user(buffer, message, size_of_message);
    memset(message, 0, sizeof(message));

    if (error_count==0)
    {
        printk(KERN_INFO "[FPGA][ C ] Sent %d characters to the user\n", size_of_message);
        return (size_of_message = 0);  // clear the position to the start and return 0
    }
    else 
    {
        printk(KERN_INFO "[FPGA][ C ] Failed to send %d characters to the user\n", error_count);
        return -EFAULT; // Failed -- return a bad address message (i.e. -14)
    }
}

#define SOFTWARE_INTERRUPT_NUMBER 0x80  // Software Interrupt Number

static int irq = SOFTWARE_INTERRUPT_NUMBER;

// Interrupt handler function
static irqreturn_t software_interrupt_handler(int irq, void *dev_id)
{
    printk(KERN_INFO "Software Interrupt Handled\n");
    return IRQ_HANDLED;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    error_count = copy_from_user(message, buffer, len);

    if (strncmp(message, "int", 3) == 0)
    {
        printk(KERN_INFO "[FPGA][ C ] Finally Got You!\n");

        // Request software interrupt
        int result = request_irq(irq, software_interrupt_handler, IRQF_SHARED, "software_interrupt", (void *)(software_interrupt_handler));
        if (result) {
            printk(KERN_ERR "Failed to request software interrupt: %d\n", result);
            return result;
        }

    }

    if (error_count==0)
    {
        size_of_message = strlen(message);
        printk(KERN_INFO "[FPGA][ C ] Received %d characters from the user\n", len);
        return len;
    } 
    else 
    {
        printk(KERN_INFO "[FPGA][ C ] Failed to receive characters from the user\n");
        return -EFAULT;
    }
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    mutex_unlock(&com_mutex);
    printk(KERN_INFO "[FPGA][ C ] Device successfully closed\n");
    return 0;
}

//////////////////////
//                  //
//                  //
//                  //
//   [SPI] Comms    //
//                  //
//                  //
//                  //
//////////////////////////////////////////////////////
//                                                  //
//                                                  //
// PIN_119 :: BBB P9_17 :: PULPLE   :: SPI0_CS0     //
// PIN_121 :: BBB P9_18 :: BLUE     :: SPI0_D1      //
// PIN_125 :: BBB P9_21 :: BROWN    :: SPI0_D0      //
// PIN_129 :: BBB P9_22 :: BLACK    :: SPI0_SCLK    //
//                                                  //
// BBB P9_28 :: YELOW    :: SPI1_CS0                //
// BBB P9_30 :: GREEN    :: SPI1_D1 :: GPIO_112     //
// BBB P9_29 :: RED      :: SPI1_D0                 //
// BBB P9_31 :: ORANGE   :: SPI1_SCLK               //
//                                                  //
//                                                  //
//////////////////////////////////////////////////////

static struct spi_device *spi_dev0;
static struct spi_device *spi_dev1;

static volatile uint8_t tx_buffer0[] = {0xAA};  // Data to be transmitted for SPI0
static volatile uint8_t rx_buffer0[1];          // Buffer to receive data for SPI0

static volatile uint8_t tx_buffer1[] = {0xBB};  // Data to be transmitted for SPI1
static volatile uint8_t rx_buffer1[1];          // Buffer to receive data for SPI1

static struct work_struct spi_response_work;
static struct workqueue_struct *spi_response_wq;

static void spi_response_func(struct work_struct *work)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    // Initialize SPI transfer for SPI0
    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = tx_buffer0;
    transfer.rx_buf = rx_buffer0;
    transfer.len = sizeof(tx_buffer0);

    // Initialize SPI messages
    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    // Transfer SPI messages for SPI0
    ret = spi_sync(spi_dev0, &msg);
    if (ret < 0) {
        printk(KERN_ERR "[FPGA][SPI] SPI transfer for SPI0 failed: %d\n", ret);
        return;
    }

    // Display the received data for SPI0
    printk(KERN_INFO "[FPGA][SPI] Received data for SPI0:");
    for (i = 0; i < sizeof(rx_buffer0); ++i) {
        printk(KERN_INFO "[FPGA][SPI] Byte %d: 0x%02x\n", i, rx_buffer0[i]);
    }
}

// static void spi_request_func(struct work_struct *work)
// {
//     struct spi_message msg;
//     struct spi_transfer transfer;
//     int ret;
//     int i;

//     // Initialize SPI transfer for SPI1
//     memset(&transfer, 0, sizeof(transfer));
//     transfer.tx_buf = tx_buffer1;  // Send received data from SPI0
//     transfer.rx_buf = rx_buffer1;  // Receive data from FPGA via SPI1
//     transfer.len = sizeof(tx_buffer0);

//     // Initialize SPI messages
//     spi_message_init(&msg);
//     spi_message_add_tail(&transfer, &msg);

//     // Transfer SPI messages for SPI1
//     ret = spi_sync(spi_dev1, &msg);
//     if (ret < 0) {
//         printk(KERN_ERR "[FPGA][SPI] SPI transfer for SPI1 failed: %d\n", ret);
//         return;
//     }

//     // Display the received data for SPI1
//     printk(KERN_INFO "[FPGA][SPI] Received data for SPI1:");
//     for (i = 0; i < sizeof(rx_buffer1); ++i) {
//         printk(KERN_INFO "[FPGA][SPI] Byte %d: 0x%02x\n", i, rx_buffer1[i]);
//     }
// }

//////////////////////////
//                      //
//                      //
//                      //
//        [GPIO]        //
//      Interrupts      //
//                      //
//                      //
//                      //
//////////////////////////

#define GPIO_RESPONSE_PIN 60 // P9_12
#define GPIO_RESPONSE "GPIO_RESPONSE"

static irqreturn_t isr_response(int irq, void *data)
{
    static int counter = 0;

    //////////////////
    //              //
    //              //
    //              //
    //     FPGA     //
    //     COMS     //
    //   ISR CODE   //
    //              //
    //              //
    //              //
    //////////////////

    printk(KERN_INFO "[FPGA][ISR] Resonse interrupt [%d] @ Pin [%d]\n", counter, GPIO_RESPONSE_PIN);
    counter++;

    queue_work(spi_response_wq, &spi_response_work);

    return IRQ_HANDLED;
}

//////////////////////////
//                      //
//                      //
//                      //
//        [FPGA]        //
//        DRIVER        //
//         INIT         //
//                      //
//                      //
//                      //
//////////////////////////

static int __init fpga_driver_init(void)
{
    //////////////////////////////////
    //                              //
    // SPI :: CONFIG                //
    //                              //
    //////////////////////////////////
    struct spi_master *spi_master0;
    struct spi_master *spi_master1;
    int ret;

    spi_master0 = spi_busnum_to_master(0);
    if (!spi_master0) {
        printk(KERN_ERR "[FPGA][SPI] SPI master for SPI0 not found\n");
        return -ENODEV;
    }

    spi_master1 = spi_busnum_to_master(1);
    if (!spi_master1) {
        printk(KERN_ERR "[FPGA][SPI] SPI master for SPI1 not found\n");
        return -ENODEV;
    }

    // Prepare the SPI devices
    spi_dev0 = spi_alloc_device(spi_master0);
    if (!spi_dev0) {
        printk(KERN_ERR "[FPGA][SPI] Failed to allocate SPI device for SPI0\n");
        return -ENOMEM;
    }

    spi_dev1 = spi_alloc_device(spi_master1);
    if (!spi_dev1) {
        printk(KERN_ERR "[FPGA][SPI] Failed to allocate SPI device for SPI1\n");
        spi_dev_put(spi_dev0);
        return -ENOMEM;
    }

    spi_dev0->chip_select = 0;
    spi_dev0->mode = SPI_MODE_0;
    spi_dev0->bits_per_word = 8;
    spi_dev0->max_speed_hz = 1000000;

    spi_dev1->chip_select = 0;
    spi_dev1->mode = SPI_MODE_0;
    spi_dev1->bits_per_word = 8;
    spi_dev1->max_speed_hz = 1000000;

    ret = spi_setup(spi_dev0);
    if (ret < 0) {
        printk(KERN_ERR "[FPGA][SPI] Failed to setup SPI device for SPI0: %d\n", ret);
        spi_dev_put(spi_dev0);
        spi_dev_put(spi_dev1);
        return ret;
    }

    ret = spi_setup(spi_dev1);
    if (ret < 0) {
        printk(KERN_ERR "[FPGA][SPI] Failed to setup SPI device for SPI1: %d\n", ret);
        spi_dev_put(spi_dev0);
        spi_dev_put(spi_dev1);
        return ret;
    }

    INIT_WORK(&spi_response_work, spi_response_func);
    spi_response_wq = create_singlethread_workqueue("spi_workqueue");
    if (!spi_response_wq) {
        printk(KERN_ERR "[FPGA][WRK] Failed to create SPI workqueue\n");
        return -ENOMEM;
    }

    //////////////////////////////////
    //                              //
    // ISR :: CONFIG                //
    //                              //
    //////////////////////////////////
    int irq, result;

    result = gpio_request(GPIO_RESPONSE_PIN, GPIO_RESPONSE);
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to request GPIO pin\n");
        return result;
    }

    result = gpio_direction_input(GPIO_RESPONSE_PIN);
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to set GPIO direction\n");
        gpio_free(GPIO_RESPONSE_PIN);
        return result;
    }

    irq = gpio_to_irq(GPIO_RESPONSE_PIN);
    if (irq < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to get IRQ number\n");
        gpio_free(GPIO_RESPONSE_PIN);
        return irq;
    }

    result = request_irq(irq, isr_response, IRQF_TRIGGER_RISING, GPIO_RESPONSE, NULL);
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to request IRQ\n");
        gpio_free(GPIO_RESPONSE_PIN);
        spi_dev_put(spi_dev0);
        spi_dev_put(spi_dev1);
        return result;
    }

    printk(KERN_INFO "[FPGA][IRQ] ISR initialized\n");

    //////////////////////////////////
    //                              //
    // [C] Device :: CONFIG         //
    //                              //
    //////////////////////////////////
    printk(KERN_INFO "[FPGA][ C ] Device Init\n");

    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber<0)
    {
        printk(KERN_ALERT "[FPGA][ C ] Failed to register major number\n");
        return majorNumber;
    }

    C_Class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(C_Class))
    {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "[FPGA][ C ] Failed to register device class\n");
        return PTR_ERR(C_Class);
    }
    
    C_Device = device_create(C_Class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(C_Device))
    {
        class_destroy(C_Class);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "[FPGA][ C ] Failed to create the device\n");
        return PTR_ERR(C_Device);
    }

    mutex_init(&com_mutex);

    return 0;
}

//////////////////////////
//                      //
//                      //
//                      //
//        [FPGA]        //
//        DRIVER        //
//         EXIT         //
//                      //
//                      //
//                      //
//////////////////////////

static void __exit fpga_driver_exit(void)
{
    //////////////////////////////////
    //                              //
    // SPI :: CONFIG                //
    //                              //
    //////////////////////////////////
    cancel_work_sync(&spi_response_work);

    // Destroy RESPONSE workqueue
    if (spi_response_wq) {
        flush_workqueue(spi_response_wq);
        destroy_workqueue(spi_response_wq);
        spi_response_wq = NULL;
    }

    spi_dev_put(spi_dev0);
    spi_dev_put(spi_dev1);
    printk(KERN_INFO "[FPGA][SPI] Exit\n");

    //////////////////////////////////
    //                              //
    // ISR :: DESTROY                //
    //                              //
    //////////////////////////////////
    int irq = gpio_to_irq(GPIO_RESPONSE_PIN);
    free_irq(irq, NULL);
    gpio_free(GPIO_RESPONSE_PIN);
    printk(KERN_INFO "[FPGA][IRQ] Exit\n");
    free_irq(irq, (void *)(software_interrupt_handler));

    //////////////////////////////////
    //                              //
    // [C] Device :: DESTROY        //
    //                              //
    //////////////////////////////////
    device_destroy(C_Class, MKDEV(majorNumber, 0));
    class_unregister(C_Class);
    class_destroy(C_Class);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    mutex_destroy(&com_mutex);
    printk(KERN_INFO "[FPGA][ C ] Device Exit\n");
}

module_init(fpga_driver_init);
module_exit(fpga_driver_exit);