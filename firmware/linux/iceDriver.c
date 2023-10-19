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

static struct spi_device *spi_dev0;
static struct spi_device *spi_dev1;

static volatile uint8_t tx_res_buffer[] = {0xAB,0xCD};  // Data to be transmitted for SPI0
static volatile uint8_t rx_res_buffer[2];               // Buffer to receive data for SPI0

static volatile uint8_t tx_req_buffer[] = {0xDC,0xBA};  // Data to be transmitted for SPI1
static volatile uint8_t rx_req_buffer[2];               // Buffer to receive data for SPI1

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

#define GPIO_IN_SPI_INTERRUPT_PIN 60    // P9_12

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
    CAN     = 2,    
    USER    = 3
};

enum StateMachine STATE;

static struct task_struct *stm_thread;

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
                printk(KERN_INFO "[FPGA][STM] IDLE State [%d]\n",counter);
                break;

            case SPI:
                printk(KERN_INFO "[FPGA][STM] SPI State [%d]\n",counter);
                STATE = IDLE;
                break;

            case CAN:
                printk(KERN_INFO "[FPGA][STM] CAN State [%d]\n",counter);
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

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    error_count = copy_from_user(message, buffer, len);

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
static void spi_response_func(struct work_struct *work)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = tx_res_buffer;
    //
    // transfer.rx_buf = rx_res_buffer;
    transfer.len = sizeof(tx_res_buffer);

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_dev0, &msg);
    if (ret < 0) {
        printk(KERN_ERR "[FPGA][SPI] SPI transfer for SPI response: %d\n", ret);
        return;
    }

    printk(KERN_INFO "[FPGA][SPI] Received data for SPI 0 ---==[ RESPONSE ]==---");
    for (i = 0; i < sizeof(rx_res_buffer); ++i) {
        printk(KERN_INFO "[FPGA][SPI] Byte %d: 0x%02x\n", i, rx_res_buffer[i]);
    }
}

static void spi_request_func(struct work_struct *work)
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

    ret = spi_sync(spi_dev0, &msg);
    if (ret < 0) {
        printk(KERN_ERR "[FPGA][SPI] SPI transfer for SPI 0 failed: %d\n", ret);
        return;
    }

    printk(KERN_INFO "[FPGA][SPI] Received data for SPI 0 ---==[ REQUEST ]==---");
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
static irqreturn_t isr_spi_response(int irq, void *data)
{
    static int counter = 0;

    printk(KERN_INFO "[FPGA][ISR] SPI Response interrupt [%d] @ Pin [%d]\n", counter, GPIO_IN_SPI_INTERRUPT_PIN);
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
    
    ////////////////////////////////////
    //                                //
    // STATE MACHINE :: THREAD CONFIG //
    //                                //
    ////////////////////////////////////
    stm_thread = kthread_create(StateMachineThread, NULL, "stm_thread");

    if (IS_ERR(stm_thread)) {
        printk(KERN_ERR "Failed to create kernel thread\n");
        return PTR_ERR(stm_thread);
    }

    wake_up_process(stm_thread);

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
        return ret;
    }

    ret = spi_setup(spi_dev1);
    if (ret < 0) {
        printk(KERN_ERR "[FPGA][SPI] Failed to setup SPI device for SPI1: %d\n", ret);
        spi_dev_put(spi_dev1);
        return ret;
    }

    INIT_WORK(&spi_response_work, spi_response_func);
    spi_response_wq = create_singlethread_workqueue("spi_res_workqueue");
    if (!spi_response_wq) {
        printk(KERN_ERR "[FPGA][WRK] Failed to create SPI response workqueue\n");
        return -ENOMEM;
    }

    INIT_WORK(&spi_request_work, spi_request_func);
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
    int irq_can;
    int result;

    result = gpio_request(GPIO_IN_SPI_INTERRUPT_PIN, "Input GPIO SPI Interrupt");
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to request GPIO pin for SPI\n");
        return result;
    }
    result = gpio_direction_input(GPIO_IN_SPI_INTERRUPT_PIN);
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to set GPIO direction for SPI\n");
        gpio_free(GPIO_IN_SPI_INTERRUPT_PIN);
        return result;
    }
    irq_spi = gpio_to_irq(GPIO_IN_SPI_INTERRUPT_PIN);
    if (irq_spi < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to get IRQ number for SPI\n");
        gpio_free(GPIO_IN_SPI_INTERRUPT_PIN);
        return irq_spi;
    }
    result = request_irq(irq_spi, isr_spi_response, IRQF_TRIGGER_RISING, "Input GPIO SPI Interrupt", NULL);
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to request IRQ for SPI\n");
        gpio_free(GPIO_IN_SPI_INTERRUPT_PIN);
        spi_dev_put(spi_dev0);
        return result;
    }
    printk(KERN_INFO "[FPGA][IRQ] ISR for SPI initialized\n");

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

    spi_dev_put(spi_dev0);
    spi_dev_put(spi_dev1);
    printk(KERN_INFO "[FPGA][SPI] Exit\n");

    //////////////////////////////////
    //                              //
    // ISR :: DESTROY                //
    //                              //
    //////////////////////////////////
    int irq_spi;
    int irq_can;

    irq_spi = gpio_to_irq(GPIO_IN_SPI_INTERRUPT_PIN);
    free_irq(irq_spi, NULL);
    gpio_free(GPIO_IN_SPI_INTERRUPT_PIN);
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
    if (stm_thread) 
    {
        kthread_stop(stm_thread);
        stm_thread = NULL;
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