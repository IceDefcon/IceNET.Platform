//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/spi/spi.h>
#include <linux/interrupt.h>

MODULE_VERSION("3.0");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marek Ice");
MODULE_DESCRIPTION("FPGA Comms Driver");

//
// [C] Device
//
#define  DEVICE_NAME "iceCOM"
#define  CLASS_NAME  "iceCOM"

static int    majorNumber;
static char   message[256] = {0};
static short  size_of_message;
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

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    error_count = copy_from_user(message, buffer, len);

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

//
// GPIO Interrupt
//
#define GPIO_PIN 60 // P9_12
#define GPIO_DESC "GPIO_ISR"

static irqreturn_t gpio_isr(int irq, void *data)
{
    static int i = 0;

    //////////////
    //          //
    //          //
    //          //
    // ISR code //
    //          //
    //          //
    //          //
    //////////////

    printk(KERN_INFO "[FPGA][IRQ] GPIO interrupt [%d] @ Pin [%d]\n", i, GPIO_PIN);
    i++;

    // Transfer SPI messages for SPI0
    ret = spi_sync(spi_dev0, &msg0);
    if (ret < 0) {
        pr_err("SPI transfer for SPI0 failed: %d\n", ret);
        spi_dev_put(spi_dev0);
        spi_dev_put(spi_dev1);
        return ret;
    }

    // Transfer SPI messages for SPI1
    ret = spi_sync(spi_dev1, &msg1);
    if (ret < 0) {
        pr_err("SPI transfer for SPI1 failed: %d\n", ret);
        spi_dev_put(spi_dev0);
        spi_dev_put(spi_dev1);
        return ret;
    }

    // Display the received data for SPI0
    pr_info("Received data for SPI0:");
    for (int i = 0; i < sizeof(rx_buffer0); ++i) {
        pr_info("Byte %d: 0x%02x", i, rx_buffer0[i]);
    }

    // Display the received data for SPI1
    pr_info("Received data for SPI1:");
    for (int i = 0; i < sizeof(rx_buffer1); ++i) {
        pr_info("Byte %d: 0x%02x", i, rx_buffer1[i]);
    }

    return IRQ_HANDLED;
}

//
// SPI
//
static struct spi_device *spi_dev0;
static struct spi_device *spi_dev1;

static uint8_t tx_buffer0[] = {0x01, 0x02, 0x03, 0x04};  // Data to be transmitted for SPI0
static uint8_t rx_buffer0[4];                            // Buffer to receive data for SPI0

static uint8_t tx_buffer1[] = {0x05, 0x06, 0x07, 0x08};  // Data to be transmitted for SPI1
static uint8_t rx_buffer1[4];                            // Buffer to receive data for SPI1


//
// FPGA Driver
//
static int __init fpga_driver_init(void)
{
    //
    // GPIO Interrupt
    //
    int irq, result;

    // Request GPIO pin
    result = gpio_request(GPIO_PIN, GPIO_DESC);
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to request GPIO pin\n");
        return result;
    }

    // Set GPIO pin as input
    result = gpio_direction_input(GPIO_PIN);
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to set GPIO direction\n");
        gpio_free(GPIO_PIN);
        return result;
    }

    // Get IRQ number for GPIO pin
    irq = gpio_to_irq(GPIO_PIN);
    if (irq < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to get IRQ number\n");
        gpio_free(GPIO_PIN);
        return irq;
    }

    // Request IRQ for GPIO pin
    result = request_irq(irq, gpio_isr, IRQF_TRIGGER_RISING, GPIO_DESC, NULL);
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to request IRQ\n");
        gpio_free(GPIO_PIN);
        return result;
    }

    printk(KERN_INFO "[FPGA][IRQ] ISR initialized\n");

    //
    // [C] Device
    //
    printk(KERN_INFO "[FPGA][ C ] Device Init\n");

    // Try to dynamically allocate a major number for the device -- more difficult but worth it
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber<0)
    {
        printk(KERN_ALERT "[FPGA][ C ] Failed to register major number\n");
        return majorNumber;
    }

    // Register the device class
    C_Class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(C_Class)) // Check for error and clean up if there is one
    {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "[FPGA][ C ] Failed to register device class\n");
        return PTR_ERR(C_Class); // Correct way to return an error on a pointer
    }
    
    // Register the device driver
    C_Device = device_create(C_Class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(C_Device))   // Clean up if there is an error
    {
        class_destroy(C_Class);           // Repeated code but the alternative is goto statements
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "[FPGA][ C ] Failed to create the device\n");
        return PTR_ERR(C_Device);
    }

    mutex_init(&com_mutex);       // Initialize the mutex lock dynamically at runtime

    //
    // SPI
    //

    struct spi_master *spi_master0;
    struct spi_master *spi_master1;
    struct spi_message msg0;
    struct spi_message msg1;
    struct spi_transfer xfer0;
    struct spi_transfer xfer1;
    int ret;

    // Get the SPI masters
    spi_master0 = spi_busnum_to_master(0);  // SPI0 on BeagleBone Black
    if (!spi_master0) {
        pr_err("SPI master for SPI0 not found\n");
        return -ENODEV;
    }

    spi_master1 = spi_busnum_to_master(1);  // SPI1 on BeagleBone Black
    if (!spi_master1) {
        pr_err("SPI master for SPI1 not found\n");
        return -ENODEV;
    }

    // Prepare the SPI devices
    spi_dev0 = spi_alloc_device(spi_master0);
    if (!spi_dev0) {
        pr_err("Failed to allocate SPI device for SPI0\n");
        return -ENOMEM;
    }

    spi_dev1 = spi_alloc_device(spi_master1);
    if (!spi_dev1) {
        pr_err("Failed to allocate SPI device for SPI1\n");
        spi_dev_put(spi_dev0);
        return -ENOMEM;
    }

    // Configure SPI0 device
    spi_dev0->chip_select = 0;  // Set the chip select value (0 for SPI0 on BeagleBone Black)
    spi_dev0->mode = SPI_MODE_0;  // Set the SPI mode (0 for mode 0)
    spi_dev0->bits_per_word = 8;  // Set the number of bits per word

    // Configure SPI1 device
    spi_dev1->chip_select = 0;  // Set the chip select value (0 for SPI1 on BeagleBone Black)
    spi_dev1->mode = SPI_MODE_0;  // Set the SPI mode (0 for mode 0)
    spi_dev1->bits_per_word = 8;  // Set the number of bits per word

    // Setup SPI0 device
    ret = spi_setup(spi_dev0);
    if (ret < 0) {
        pr_err("Failed to setup SPI device for SPI0: %d\n", ret);
        spi_dev_put(spi_dev0);
        spi_dev_put(spi_dev1);
        return ret;
    }

    // Setup SPI1 device
    ret = spi_setup(spi_dev1);
    if (ret < 0) {
        pr_err("Failed to setup SPI device for SPI1: %d\n", ret);
        spi_dev_put(spi_dev0);
        spi_dev_put(spi_dev1);
        return ret;
    }

    // Initialize SPI transfer for SPI0
    memset(&xfer0, 0, sizeof(xfer0));
    xfer0.tx_buf = tx_buffer0;
    xfer0.rx_buf = rx_buffer0;
    xfer0.len = sizeof(tx_buffer0);

    // Initialize SPI transfer for SPI1
    memset(&xfer1, 0, sizeof(xfer1));
    xfer1.tx_buf = tx_buffer1;
    xfer1.rx_buf = rx_buffer1;
    xfer1.len = sizeof(tx_buffer1);

    // Initialize SPI messages
    spi_message_init(&msg0);
    spi_message_init(&msg1);
    spi_message_add_tail(&xfer0, &msg0);
    spi_message_add_tail(&xfer1, &msg1);

    return 0;
}

static void __exit fpga_driver_exit(void)
{
    //
    // SPI
    //
    spi_dev_put(spi_dev0);
    spi_dev_put(spi_dev1);

    //
    // IRQ
    //
    int irq = gpio_to_irq(GPIO_PIN);

    // Free IRQ for GPIO pin
    free_irq(irq, NULL);

    // Free GPIO pin
    gpio_free(GPIO_PIN);

    printk(KERN_INFO "[FPGA][IRQ] ISR exited\n");

    //
    // [C] Device
    //
    printk(KERN_INFO "[FPGA][ C ] Device Exit\n");
    device_destroy(C_Class, MKDEV(majorNumber, 0));     // remove the device
    class_unregister(C_Class);                          // unregister the device class
    class_destroy(C_Class);                             // remove the device class
    unregister_chrdev(majorNumber, DEVICE_NAME);        // unregister the major number
    mutex_destroy(&com_mutex);                          // destroy the dynamically-allocated mutex
}

module_init(fpga_driver_init);
module_exit(fpga_driver_exit);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("[FPGA][SPI] Driver");
MODULE_LICENSE("GPL");
