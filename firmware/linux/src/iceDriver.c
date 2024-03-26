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

#include "charDevice.h"
#include "workLoad.h"
#include "spiFpga.h"

MODULE_VERSION("2.0");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice Marek");
MODULE_DESCRIPTION("FPGA Comms Driver");

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

static volatile uint8_t tx_kernel[] = {0x81};
static volatile uint8_t rx_kernel[1];

static volatile uint8_t tx_fb mfpga[] = {0xC3};
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
// BBB P9_17 :: YELLOW   :: SPI0_CS0  :: CS     //
// BBB P9_18 :: ORNANGE  :: SPI0_D1   :: MOSI   //
// BBB P9_21 :: GREEN    :: SPI0_D0   :: MISO   //
// BBB P9_22 :: BLUE     :: SPI0_SCLK :: SCLK   //
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

    queue_work(get_kernel_wq(), get_kernel_work());

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
    INIT_WORK(get_kernel_work(), kernel_execute);
    set_kernel_wq(create_singlethread_workqueue("kernel_workqueue"));
    if (!get_kernel_wq()) {
        printk(KERN_ERR "[FPGA][WRK] Failed to create kernel workqueue\n");
        return -ENOMEM;
    }

    INIT_WORK(get_fpga_work(), fpga_command);
    set_fpga_wq(create_singlethread_workqueue("fpga_workqueue"));
    if (!get_fpga_wq()) {
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

    charDeviceInit(void);

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
    cancel_work_sync(get_kernel_work());
    if (get_kernel_wq()) {
        flush_workqueue(get_kernel_wq());
        destroy_workqueue(get_kernel_wq());
        set_kernel_wq(NULL);
    }

    cancel_work_sync(get_fpga_work());
    if (get_fpga_wq()) {
        flush_workqueue(get_fpga_wq());
        destroy_workqueue(get_fpga_wq());
        set_fpga_wq(NULL);
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

    charDeviceDestroy();

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