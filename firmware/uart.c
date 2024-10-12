#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>

#define DEVICE "/dev/ttyS1"
#define BAUD_RATE 921600

static struct file *uart_filp;
static struct tty_struct *uart_tty;  // Pointer to the TTY structure

static void uart_close(void);

static int uart_set_baud_rate(struct tty_struct *tty, int baud_rate)
{
    struct ktermios *termios;   // Terminal IO structure

    // Get the current terminal I/O settings
    termios = &tty->termios;

    // Set baud rate (input and output speeds)
    termios->c_cflag &= ~CBAUD;  // Clear current baud rate
    termios->c_cflag |= BOTHER;  // Use custom baud rate option

    // Specify the baud rate
    tty_termios_encode_baud_rate(termios, baud_rate, baud_rate);

    // Apply the new settings to the tty
    tty_set_termios(tty, termios);

    printk(KERN_INFO "Baud rate set to %d.\n", baud_rate);
    return 0;
}

static struct tty_struct* get_tty_from_file(struct file *filp)
{
    struct tty_file_private *priv;
    struct tty_struct *tty;

    if (!filp || !filp->private_data)
    {
        return NULL;
    }

    priv = filp->private_data;
    tty = priv->tty;

    return tty;
}

static int uart_open(void)
{
    int ret;

    // Open the UART device
    uart_filp = filp_open(DEVICE, O_RDWR | O_NOCTTY | O_NDELAY, 0);
    if (IS_ERR(uart_filp)) {
        printk(KERN_ERR "Failed to open UART device: %ld\n", PTR_ERR(uart_filp));
        return PTR_ERR(uart_filp);
    }

    printk(KERN_INFO "UART device opened successfully.\n");

    // Get the tty structure from the file
    uart_tty = get_tty_from_file(uart_filp);
    if (!uart_tty) {
        printk(KERN_ERR "Failed to get tty from file.\n");
        uart_close();
        return -ENODEV;
    }

    // Set the desired baud rate (max acceptable)
    ret = uart_set_baud_rate(uart_tty, BAUD_RATE);
    if (ret < 0) {
        printk(KERN_ERR "Failed to set baud rate.\n");
        uart_close();
        return ret;
    }

    return 0;
}

static void uart_close(void)
{
    if (!IS_ERR(uart_filp)) {
        filp_close(uart_filp, NULL);
        printk(KERN_INFO "UART device closed.\n");
    }
}

static ssize_t uart_write(const char *buf, size_t len)
{
    mm_segment_t oldfs;
    ssize_t ret;

    if (!uart_filp) {
        printk(KERN_ERR "UART device is not open!\n");
        return -ENODEV;
    }

    oldfs = get_fs();
    set_fs(KERNEL_DS);  // Change address limit for kernel space write

    ret = vfs_write(uart_filp, buf, len, &uart_filp->f_pos);  // Write to UART

    set_fs(oldfs);  // Restore address limit

    if (ret < 0)
    {
        printk(KERN_ERR "UART write failed: %d\n", ret);  // Changed %ld to %d
    }
    else
    {
        printk(KERN_INFO "Sent to UART: %s\n", buf);
    }

    return ret;
}

static ssize_t uart_write(const char *buf, size_t len)
{
    mm_segment_t oldfs;
    ssize_t ret;
    int retries = 3;

    if (!uart_filp)
    {
        printk(KERN_ERR "UART device is not open!\n");
        return -ENODEV;
    }

    oldfs = get_fs();
    set_fs(KERNEL_DS);

    while (retries--)
    {
        ret = vfs_write(uart_filp, buf, len, &uart_filp->f_pos);
        if (ret != -EAGAIN)
        {
            break;
        }
        printk(KERN_INFO "UART write failed, number of retries left: %d\n", retries);
        msleep(10);  // Delay before retrying
    }

    set_fs(oldfs);

    if (ret < 0)
    {
        printk(KERN_ERR "UART write failed: %zd\n", ret);
    }
    else
    {
        printk(KERN_INFO "Sent to UART: %s\n", buf);
    }

    return ret;
}


static int __init uart_module_init(void)
{
    int ret;

    // Open the UART device
    ret = uart_open();
    if (ret < 0)
    {
        return ret;
    }

    // Add a delay to ensure UART is ready
    msleep(100);  // Sleep for 100ms

    // Write some data to UART
    uart_write("Hello, UART!\n", 13);

    return 0;
}

static void __exit uart_module_exit(void)
{
    // Close the UART device
    uart_close();
}

module_init(uart_module_init);
module_exit(uart_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice Marek");
MODULE_DESCRIPTION("UART Driver");
