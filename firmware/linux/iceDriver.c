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
#include <linux/kthread.h>
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
//   [W] Workload   //
//                  //
//                  //
//                  //
//////////////////////
static struct work_struct spi_request_work;
static struct work_struct spi_response_work;
static struct workqueue_struct *spi_request_wq;
static struct workqueue_struct *spi_response_wq;

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

//////////////////////
//                  //
//                  //
//                  //
//   [SPI] Comms    //
//                  //
//                  //
//                  //
//////////////////////////////////////////
//                                      //
//                                      //
// BBB P9_17 :: PULPLE   :: SPI0_CS0    //
// BBB P9_18 :: BLUE     :: SPI0_D1     //
// BBB P9_21 :: BROWN    :: SPI0_D0     //
// BBB P9_22 :: BLACK    :: SPI0_SCLK   //
//                                      //
// BBB P9_28 :: YELOW    :: SPI1_CS0    //
// BBB P9_30 :: GREEN    :: SPI1_D1     //
// BBB P9_29 :: RED      :: SPI1_D0     //
// BBB P9_31 :: ORANGE   :: SPI1_SCLK   //
//                                      //
//                                      //
//////////////////////////////////////////

static struct spi_device *spi_dev;

static volatile uint8_t tx_res_buffer[] = {0x11};       // Data to be transmitted for SPI0
static volatile uint8_t rx_res_buffer[1];               // Buffer to receive data for SPI0

static volatile uint8_t tx_req_buffer[] = {0x22};       // Data to be transmitted for SPI0
static volatile uint8_t rx_req_buffer[1];               // Buffer to receive data for SPI0

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

#define GPIO_IN_INTERRUPT_SPI 60    // P9_12
#define GPIO_IN_INTERRUPT_API 50    // P9_14

///////////////////
//               //
//               //
//               //
// State Machine //
//               //
//               //
//               //
///////////////////

enum StateMachine 
{
    IDLE    = 0,
    SPI     = 1,
    I2C     = 2,    
    USER    = 3
};

enum StateMachine STATE;

static struct task_struct *thread_handle;

//////////////////////////////////////////////////////////////
//                                                          //
////                                                      ////
//////  C   O   D   E   :::  S   E   C   T   I   O   N  //////
////                                                      ////
//                                                          //
//////////////////////////////////////////////////////////////

///////////////////
//               //
//               //
//               //
// State Machine //
//               //
//               //
//               //
///////////////////


int StateMachineThread(void *data)
{
    int counter = 0;

    STATE = IDLE;

    while (!kthread_should_stop()) 
    {
        switch(STATE)
        {
            case IDLE:
                // printk(KERN_INFO "[FPGA][STM] Idle State [%d]\n",counter);
                break;

            case SPI:
                printk(KERN_INFO "[FPGA][STM] SPI State [%d]\n",counter);
                STATE = IDLE;
                break;

            case I2C:
                printk(KERN_INFO "[FPGA][STM] I2C State [%d]\n",counter);
                STATE = IDLE;
                break;

            case USER:
                printk(KERN_INFO "[FPGA][STM] USER State [%d]\n",counter);
                STATE = IDLE;
                break;

            default:
                printk(KERN_INFO "[FPGA][STM] Unknown State [%d]\n",counter);
                break;
        }

        msleep(1000);  // Delay for 1 second
        counter++;
    }

    return 0;
}

//////////////////////
//                  //
//                  //
//                  //
//    [C] Device    //
//                  //
//                  //
//                  //
//////////////////////
static int dev_open(struct inode *inodep, struct file *filep)
{
    if(!mutex_trylock(&com_mutex))
    {
        printk(KERN_ALERT "[FPGA][ C ] Device in use by another process");
        return -EBUSY;
    }

    numberOpens++;
    printk(KERN_INFO "[FPGA][ C ] Device has been opened %d time(s)\n", numberOpens);
    return NULL;
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
        return (size_of_message = 0);  // clear the position to the start and return NULL
    }
    else 
    {
        printk(KERN_INFO "[FPGA][ C ] Failed to send %d characters to the user\n", error_count);
        return -EFAULT; // Failed -- return a bad address message (i.e. -14)
    }
}

struct Move
{
    int Left;
    int Right;
    int Up;
    int Down;
}


static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    error_count = copy_from_user(message, buffer, len);

    switch(message)
    {
        case "L":
        printk(KERN_INFO "[FPGA][ C ] Left \n");

        case "R":
        printk(KERN_INFO "[FPGA][ C ] Right \n");

        case "U":
        printk(KERN_INFO "[FPGA][ C ] Up \n");

        case "D":
        printk(KERN_INFO "[FPGA][ C ] Down \n");
    }


    if (strncmp(message, "L", 1) == 0)


    if (strncmp(message, "int", 3) == 0)
    {
        queue_work(spi_request_wq, &spi_request_work);
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
    return NULL;
}

//////////////////////
//                  //
//                  //
//                  //
//   [SPI] Comms    //
//                  //
//                  //
//                  //
//////////////////////////////////////////////////
//                                              //
//                                              //
// BBB P9_17 :: PULPLE   :: SPI0_CS0            //
// BBB P9_18 :: BLUE     :: SPI0_D1             //
// BBB P9_21 :: BROWN    :: SPI0_D0             //
// BBB P9_22 :: BLACK    :: SPI0_SCLK           //
//                                              //
// BBB P9_28 :: YELOW    :: SPI1_CS0  :: NA     //
// BBB P9_30 :: GREEN    :: SPI1_D1   :: NA     //
// BBB P9_29 :: RED      :: SPI1_D0   :: NA     //
// BBB P9_31 :: ORANGE   :: SPI1_SCLK :: NA     //
//                                              //
//                                              //
//////////////////////////////////////////////////
static void spi_response_execute(struct work_struct *work)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = tx_res_buffer;
    transfer.rx_buf = rx_res_buffer;
    transfer.len = sizeof(tx_res_buffer);

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_dev, &msg);
    if (ret < 0) {
        printk(KERN_ERR "[FPGA][SPI] SPI transfer for SPI response failed: %d\n", ret);
        return;
    }

    printk(KERN_INFO "[FPGA][SPI] Received data for SPI ---==[ RESPONSE ]==---");
    for (i = 0; i < sizeof(rx_res_buffer); ++i) {
        printk(KERN_INFO "[FPGA][SPI] Byte %d: 0x%02x\n", i, rx_res_buffer[i]);
    }
}

static void spi_request_execute(struct work_struct *work)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = tx_req_buffer;
    transfer.rx_buf = rx_req_buffer;
    transfer.len = sizeof(tx_req_buffer);

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_dev, &msg);
    if (ret < 0) {
        printk(KERN_ERR "[FPGA][SPI] SPI transfer for SPI request failed: %d\n", ret);
        return;
    }

    printk(KERN_INFO "[FPGA][SPI] Received data for SPI ---==[ REQUEST ]==---");
    for (i = 0; i < sizeof(rx_req_buffer); ++i) {
        printk(KERN_INFO "[FPGA][SPI] Byte %d: 0x%02x\n", i, rx_req_buffer[i]);
    }
}

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
static irqreturn_t isr_spi(int irq, void *data)
{
    static int counter = 0;

    printk(KERN_INFO "[FPGA][ISR] SPI Response interrupt [%d] @ Pin [%d]\n", counter, GPIO_IN_INTERRUPT_SPI);
    counter++;

    queue_work(spi_response_wq, &spi_response_work);

    return IRQ_HANDLED;
}

static irqreturn_t isr_api(int irq, void *data)
{
    static int counter = 0;

    printk(KERN_INFO "[FPGA][ISR] API Response interrupt [%d] @ Pin [%d]\n", counter, GPIO_IN_INTERRUPT_API);
    counter++;

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
    
    ////////////////////////////////////
    //                                //
    // STATE MACHINE :: THREAD CONFIG //
    //                                //
    ////////////////////////////////////
    thread_handle = kthread_create(StateMachineThread, NULL, "thread_handle");

    if (IS_ERR(thread_handle)) {
        printk(KERN_ERR "Failed to create kernel thread\n");
        return PTR_ERR(thread_handle);
    }

    wake_up_process(thread_handle);

    //////////////////////////////////
    //                              //
    // SPI :: CONFIG                //
    //                              //
    //////////////////////////////////
    struct spi_master *spi_master0;
    int ret;

    spi_master0 = spi_busnum_to_master(0);
    if (!spi_master0) {
        printk(KERN_ERR "[FPGA][SPI] SPI master for SPI0 not found\n");
        return -ENODEV;
    }

    // Prepare the SPI devices
    spi_dev = spi_alloc_device(spi_master0);
    if (!spi_dev) {
        printk(KERN_ERR "[FPGA][SPI] Failed to allocate SPI device for SPI0\n");
        return -ENOMEM;
    }

    /*! 
     * The mode is set to 1 to pass the
     * High clock control signal to FPGA
     */
    spi_dev->chip_select = 0;
    spi_dev->mode = SPI_MODE_1;
    spi_dev->bits_per_word = 8;
    spi_dev->max_speed_hz = 1000000;

    ret = spi_setup(spi_dev);
    if (ret < 0) {
        printk(KERN_ERR "[FPGA][SPI] Failed to setup SPI device for SPI0: %d\n", ret);
        spi_dev_put(spi_dev);
        return ret;
    }

    /*!
     * Work config for
     * SPI operations
     * Request and Response
     */
    INIT_WORK(&spi_response_work, spi_response_execute);
    spi_response_wq = create_singlethread_workqueue("spi_res_workqueue");
    if (!spi_response_wq) {
        printk(KERN_ERR "[FPGA][WRK] Failed to create SPI response workqueue\n");
        return -ENOMEM;
    }

    INIT_WORK(&spi_request_work, spi_request_execute);
    spi_request_wq = create_singlethread_workqueue("spi_req_workqueue");
    if (!spi_request_wq) {
        printk(KERN_ERR "[FPGA][WRK] Failed to create SPI request workqueue\n");
        return -ENOMEM;
    }

    //////////////////////////////////
    //                              //
    // GPIO ISR :: CONFIG           //
    //                              //
    //////////////////////////////////
    int irq_spi;
    int irq_api;
    int result;

    //
    // SPI Interrupt
    //
    result = gpio_request(GPIO_IN_INTERRUPT_SPI, "Input GPIO SPI Interrupt");
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to request GPIO pin for SPI\n");
        return result;
    }
    result = gpio_direction_input(GPIO_IN_INTERRUPT_SPI);
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to set GPIO direction for SPI\n");
        gpio_free(GPIO_IN_INTERRUPT_SPI);
        return result;
    }
    irq_spi = gpio_to_irq(GPIO_IN_INTERRUPT_SPI);
    if (irq_spi < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to get IRQ number for SPI\n");
        gpio_free(GPIO_IN_INTERRUPT_SPI);
        return irq_spi;
    }
    result = request_irq(irq_spi, isr_spi, IRQF_TRIGGER_RISING, "Input GPIO SPI Interrupt", NULL);
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to request IRQ for SPI\n");
        gpio_free(GPIO_IN_INTERRUPT_SPI);
        spi_dev_put(spi_dev);
        return result;
    }
    printk(KERN_INFO "[FPGA][IRQ] ISR for SPI initialized\n");

    //
    // API Interrupt
    //
    result = gpio_request(GPIO_IN_INTERRUPT_API, "Input GPIO API Interrupt");
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to request GPIO pin for API\n");
        return result;
    }
    result = gpio_direction_input(GPIO_IN_INTERRUPT_API);
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to set GPIO direction for API\n");
        gpio_free(GPIO_IN_INTERRUPT_API);
        return result;
    }
    irq_api = gpio_to_irq(GPIO_IN_INTERRUPT_API);
    if (irq_api < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to get IRQ number for API\n");
        gpio_free(GPIO_IN_INTERRUPT_API);
        return irq_api;
    }
    result = request_irq(irq_api, isr_api, IRQF_TRIGGER_RISING, "Input GPIO API Interrupt", NULL);
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to request IRQ for API\n");
        gpio_free(GPIO_IN_INTERRUPT_API);
        return result;
    }
    printk(KERN_INFO "[FPGA][IRQ] ISR for API initialized\n");

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

    return NULL;
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
    if (spi_response_wq) {
        flush_workqueue(spi_response_wq);
        destroy_workqueue(spi_response_wq);
        spi_response_wq = NULL;
    }

    cancel_work_sync(&spi_request_work);
    if (spi_request_wq) {
        flush_workqueue(spi_request_wq);
        destroy_workqueue(spi_request_wq);
        spi_request_wq = NULL;
    }

    spi_dev_put(spi_dev);
    printk(KERN_INFO "[FPGA][SPI] Exit\n");

    //////////////////////////////////
    //                              //
    // ISR :: DESTROY               //
    //                              //
    //////////////////////////////////
    int irq_spi;
    int irq_api;

    irq_spi = gpio_to_irq(GPIO_IN_INTERRUPT_SPI);
    free_irq(irq_spi, NULL);
    gpio_free(GPIO_IN_INTERRUPT_SPI);

    irq_api = gpio_to_irq(GPIO_IN_INTERRUPT_API);
    free_irq(irq_api, NULL);
    gpio_free(GPIO_IN_INTERRUPT_API);

    printk(KERN_INFO "[FPGA][IRQ] SPI Exit\n");

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

    //////////////////////////////////
    //                              //
    // State Machine :: DESTROY     //
    //                              //
    //////////////////////////////////
    if (thread_handle) 
    {
        kthread_stop(thread_handle);
        thread_handle = NULL;
    }
    printk(KERN_INFO "[FPGA][STM] State Machine Exit\n");
}

module_init(fpga_driver_init);
module_exit(fpga_driver_exit);

//////////////////////////////////////////////////////////
//                                                      //
//                                                      //
// I2C :: Interface                                     //
//                                                      //
//                                                      //
//////////////////////////////////////////////////////////
//                                                      //
// Protocol Selectio                                    //
// For I2C ---> CS must be connected to Vdd             //
//                                                      //
// ID = 0x68 when SA0 = GND                             //
// ID = 0x69 when SA0 = Vdd                             //
//                                                      //
//                                                      //
//                                                      //
//                                                      //
//                                                      //
//////////////////////////////////////////////////////////
 
//////////////////////////////////////////////////////////
//                                                      //
//                                                      //
// CAN :: Interface                                     //
//                                                      //
//                                                      //
//////////////////////////////////////////////////////////
//                                                      //
// config-pin P9_24 can                                 //
// config-pin P9_26 can                                 //
// config-pin P9_19 can                                 //
// config-pin P9_20 can                                 //
//                                                      //
// sudo ip link set can0 up type can bitrate 500000     //
// sudo ip link set can1 up type can bitrate 500000     //
// candump can1                                         //
// cansend can0 123#1122334455667788                    //
//                                                      //
// Yellow   :: Rx   :: P9_24                            //
// Orange   :: Tx   :: P9_26                            //
// Grey     :: GND  :: P9_2                             //
// White    :: 3.3V :: P9_4                             //
//                                                      //
// Yellow   :: Rx   :: P9_19                            //
// Green    :: Tx   :: P9_20                            //
// Orange   :: GND  :: P9_1                             //
// Red      :: 3.3V :: P9_3                             //
//                                                      //
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
//                                                      //
//                                                      //
// 4D-PAM5 :: Encoder                                   //
//                                                      //
//                                                      //
//////////////////////////////////////////////////////////
//                                                      //
//                                                      //
// 1Ghz ADC                                             //
//                                                      //
//                                                      //
//                                                      //
//                                                      //
//                                                      //
//                                                      //
//////////////////////////////////////////////////////////