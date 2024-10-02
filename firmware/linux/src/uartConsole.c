/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2024
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>

#define SERIAL_DEVICE "/dev/ttyS1"
#define BAUD_RATE 921600

static struct file *uart_file_ptr;
static struct tty_struct *uart_tty_ptr;  // Pointer to the TTY structure

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

    /**
     * O_RDWR 	:: Read/Write access
     * O_NOCTTY :: No Controlling Terminal
     * O_NDELAY :: Non-blocking I/O
     */
    uart_file_ptr = filp_open(SERIAL_DEVICE, O_RDWR | O_NOCTTY | O_NDELAY, 0);
    if (IS_ERR(uart_file_ptr))
    {
        printk(KERN_ERR "[INIT][SER] Failed to open serial device: %ld\n", PTR_ERR(uart_file_ptr));
        return PTR_ERR(uart_file_ptr);
    }

    printk(KERN_INFO "[INIT][SER] Serial device opened successfully @ %s\n", SERIAL_DEVICE);

    // Get the tty structure from the file
    uart_tty_ptr = get_tty_from_file(uart_file_ptr);
    if (!uart_tty_ptr)
    {
        printk(KERN_ERR "[INIT][SER] Failed to get tty from file.\n");
        uart_close();
        return -ENODEV;
    }

    // Set the desired baud rate (max acceptable)
    ret = uart_set_baud_rate(uart_tty_ptr, BAUD_RATE);
    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][SER] Failed to set baud rate.\n");
        uart_close();
        return ret;
    }

    return 0;
}

static void uart_close(void)
{
    if (!IS_ERR(uart_file_ptr))
    {
        filp_close(uart_file_ptr, NULL);
        printk(KERN_INFO "[DESTROY][SER] UART device closed.\n");
    }
}

static ssize_t uart_write(const char *buf, size_t len)
{
    mm_segment_t oldfs;
    ssize_t ret;

    if (!uart_file_ptr)
    {
        printk(KERN_ERR "[INIT][SER] UART device is not open!\n");
        return -ENODEV;
    }

    oldfs = get_fs();
    set_fs(KERNEL_DS);  // Change address limit for kernel space write

    ret = vfs_write(uart_file_ptr, buf, len, &uart_file_ptr->f_pos);  // Write to UART

    set_fs(oldfs);  // Restore address limit

    if (ret < 0)
    {
        printk(KERN_ERR "[CTRL][SER] UART write failed: %d\n", ret);  // Changed %ld to %d
    }
    else
    {
        printk(KERN_INFO "[CTRL][SER] Sent to UART: %s\n", buf);
    }

    return ret;
}

int uartConsoleInit(void)
{
    int ret;

    ret = uart_open();
    if (ret < 0)
    {
        return ret;
    }

    /* Test Message */
    uart_write("---==[ This message have 51 ASCII characters ]==---\n", 51);

    return 0;
}

void uartConsoleDestroy(void)
{
    // Close the UART device
    uart_close();
}
