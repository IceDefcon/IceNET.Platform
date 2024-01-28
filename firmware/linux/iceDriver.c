/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
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

/* Direction commands */
// struct Direction 
// {
//     bool Left;
//     bool Right;
//     bool Up;
//     bool Down;
//     bool Go;
// };

// static struct Direction Move;

//////////////////////
//                  //
//                  //
//                  //
//   [W] Workload   //
//                  //
//                  //
//                  //
//////////////////////
static struct work_struct fpga_work;
static struct work_struct kernel_work;
static struct workqueue_struct *fpga_wq;
static struct workqueue_struct *kernel_wq;

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

static volatile uint8_t tx_kernel[] = {0xE3};
static volatile uint8_t rx_kernel[1];

static volatile uint8_t tx_fpga[] = {0xE3};
static volatile uint8_t rx_fpga[1];

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

#define GPIO_KERNEL_INTERRUPT 60    // P9_12

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

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    error_count = copy_from_user(message, buffer, len);

    if (strncmp(message, "int", 3) == 0)
    {
        queue_work(fpga_wq, &fpga_work);
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

/*!
 * 
 * Experimental Drone control
 * 
 */
// static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
// {
//     int error_count = 0;
//     error_count = copy_from_user(message, buffer, len);

//     memset(&Move, 0, sizeof(struct Direction));

//     switch (message[0]) {
//         case 'w':
//             Move.Up = true;
//             Move.Go = true;
//             break;

//         case 's':
//             Move.Down = true;
//             Move.Go = true;
//             break;

//         case 'a':
//             Move.Left = true;
//             Move.Go = true;
//             break;

//         case 'd':
//             Move.Right = true;
//             Move.Go = true;
//             break;

//         default:
//             break;
//     }

//     if (Move.Go)
//     {
//         Move.Go = false;
//         queue_work(fpga_wq, &fpga_work);
//     }

//     if (error_count==0)
//     {
//         size_of_message = strlen(message);
//         printk(KERN_INFO "[FPGA][ C ] Received %d characters from the user\n", len);
//         return len;
//     } 
//     else 
//     {
//         printk(KERN_INFO "[FPGA][ C ] Failed to receive characters from the user\n");
//         return -EFAULT;
//     }
// }

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
static void kernel_execute(struct work_struct *work)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = tx_kernel;
    transfer.rx_buf = rx_kernel;
    transfer.len = sizeof(tx_kernel);

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_dev, &msg);
    if (ret < 0) {
        printk(KERN_ERR "[FPGA][SPI] SPI transfer for Kernel failed: %d\n", ret);
        return;
    }

    printk(KERN_INFO "[FPGA][SPI] Command Data for Kernel Processing");
    for (i = 0; i < sizeof(rx_kernel); ++i) {
        printk(KERN_INFO "[FPGA][SPI] Byte %d: 0x%02x\n", i, rx_kernel[i]);
    }

    /*!
     * 
     * 
     * 
     * Here we should receive data from
     * FPGA for kernel processing
     * 
     * 
     * 
     */
}

static void fpga_command(struct work_struct *work)
{
    struct spi_message msg;
    struct spi_transfer transfer;
    int ret;
    int i;

    /* Error Value */
    // tx_fpga[0] = 0xFF;

    /* Direction commands to the FPGA */
    // if(Move.Up) tx_fpga[0] = 0x18;
    // if(Move.Down) tx_fpga[0] = 0x24;
    // if(Move.Left) tx_fpga[0] = 0x42;
    // if(Move.Right) tx_fpga[0] = 0x81;

    memset(&transfer, 0, sizeof(transfer));
    transfer.tx_buf = tx_fpga;
    transfer.rx_buf = rx_fpga;
    transfer.len = sizeof(tx_fpga);

    spi_message_init(&msg);
    spi_message_add_tail(&transfer, &msg);

    ret = spi_sync(spi_dev, &msg);
    if (ret < 0) {
        printk(KERN_ERR "[FPGA][SPI] SPI transfer for FPGA failed: %d\n", ret);
        return;
    }

    printk(KERN_INFO "[FPGA][SPI] Feedback Data for Kernel Processing");
    for (i = 0; i < sizeof(rx_fpga); ++i) {
        printk(KERN_INFO "[FPGA][SPI] Byte %d: 0x%02x\n", i, rx_fpga[i]);
    }

    /*!
     * 
     * 
     * 
     * Here we should receive movement
     * feedback data for kernel processing
     * 
     * 
     * 
     */

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
static irqreturn_t isr_kernel(int irq, void *data)
{
    static int counter = 0;

    printk(KERN_INFO "[FPGA][ISR] Kernel interrupt [%d] @ Pin [%d]\n", counter, GPIO_KERNEL_INTERRUPT);
    counter++;

    queue_work(kernel_wq, &kernel_work);

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
        printk(KERN_ERR "[FPGA][SPI] Failed to setup SPI device: %d\n", ret);
        spi_dev_put(spi_dev);
        return ret;
    }

    /*!
     * Work config for
     * SPI operations
     * Kernel and Fpga
     */
    INIT_WORK(&kernel_work, kernel_execute);
    kernel_wq = create_singlethread_workqueue("kernel_workqueue");
    if (!kernel_wq) {
        printk(KERN_ERR "[FPGA][WRK] Failed to create kernel workqueue\n");
        return -ENOMEM;
    }

    INIT_WORK(&fpga_work, fpga_command);
    fpga_wq = create_singlethread_workqueue("fpga_workqueue");
    if (!fpga_wq) {
        printk(KERN_ERR "[FPGA][WRK] Failed to create fpga workqueue\n");
        return -ENOMEM;
    }

    //////////////////////////////////
    //                              //
    // GPIO ISR :: CONFIG           //
    //                              //
    //////////////////////////////////
    int irq_kernel;
    int result;

    //
    // SPI Interrupt
    //
    result = gpio_request(GPIO_KERNEL_INTERRUPT, "   Request");
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed GPIO Request :: Pin [%d]\n", GPIO_KERNEL_INTERRUPT);
        return result;
    }
    result = gpio_direction_input(GPIO_KERNEL_INTERRUPT);
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to set GPIO direction :: Pin [%d]\n", GPIO_KERNEL_INTERRUPT);
        gpio_free(GPIO_KERNEL_INTERRUPT);
        return result;
    }
    irq_kernel = gpio_to_irq(GPIO_KERNEL_INTERRUPT);
    if (irq_kernel < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to get IRQ number :: Pin [%d]\n", GPIO_KERNEL_INTERRUPT);
        gpio_free(GPIO_KERNEL_INTERRUPT);
        return irq_kernel;
    }
    result = request_irq(irq_kernel, isr_kernel, IRQF_TRIGGER_RISING, "Request IRQ", NULL);
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to request IRQ number :: Pin [%d]\n", GPIO_KERNEL_INTERRUPT);
        gpio_free(GPIO_KERNEL_INTERRUPT);
        spi_dev_put(spi_dev);
        return result;
    }
    printk(KERN_INFO "[FPGA][IRQ] Initialized\n");

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
    cancel_work_sync(&kernel_work);
    if (kernel_wq) {
        flush_workqueue(kernel_wq);
        destroy_workqueue(kernel_wq);
        kernel_wq = NULL;
    }

    cancel_work_sync(&fpga_work);
    if (fpga_wq) {
        flush_workqueue(fpga_wq);
        destroy_workqueue(fpga_wq);
        fpga_wq = NULL;
    }

    spi_dev_put(spi_dev);
    printk(KERN_INFO "[FPGA][SPI] Exit\n");

    //////////////////////////////////
    //                              //
    // ISR :: DESTROY               //
    //                              //
    //////////////////////////////////
    int irq_kernel;

    irq_kernel = gpio_to_irq(GPIO_KERNEL_INTERRUPT);
    free_irq(irq_kernel, NULL);
    gpio_free(GPIO_KERNEL_INTERRUPT);
     printk(KERN_INFO "[FPGA][IRQ] Exit\n");

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