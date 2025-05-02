/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/ktime.h>

#include "console.h"
#include "memory.h"

#define SERIAL_DEVICE "/dev/ttyS1"
#define BAUD_RATE 921600 /* Bits per second */

static consoleProcess Process =
{
    .uartFile = NULL,
    .threadHandle = NULL,
    .stateMutex = __MUTEX_INITIALIZER(Process.stateMutex),
    .threadName = "iceConsole",
};

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

    printk(KERN_INFO "[INIT][CON] Baud rate set to %d.\n", baud_rate);
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
    struct tty_struct *uartTty;
    int ret;

    /**
     * O_RDWR 	:: Read/Write access
     * O_NOCTTY :: No Controlling Terminal
     * O_NDELAY :: Non-blocking I/O
     */
    Process.uartFile = filp_open(SERIAL_DEVICE, O_RDWR | O_NOCTTY | O_NDELAY, 0);
    if (IS_ERR(Process.uartFile))
    {
        printk(KERN_ERR "[INIT][CON] Failed to open serial device: %ld\n", PTR_ERR(Process.uartFile));
        return PTR_ERR(Process.uartFile);
    }

    printk(KERN_INFO "[INIT][CON] Serial device opened successfully @ %s\n", SERIAL_DEVICE);

    // Get the tty structure from the file
    uartTty = get_tty_from_file(Process.uartFile);
    if (!uartTty)
    {
        printk(KERN_ERR "[INIT][CON] Failed to get tty from file.\n");
        uart_close();
        return -ENODEV;
    }

    // Set the desired baud rate (max acceptable)
    ret = uart_set_baud_rate(uartTty, BAUD_RATE);
    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][CON] Failed to set baud rate.\n");
        uart_close();
        return ret;
    }

    return 0;
}

static void uart_close(void)
{
    if (!IS_ERR(Process.uartFile))
    {
        filp_close(Process.uartFile, NULL);
        printk(KERN_INFO "[DESTROY][CON] UART device closed.\n");
    }
}

ssize_t uart_write(const char *buf, size_t len)
{
    mm_segment_t oldfs;
    ssize_t ret;

    if (!Process.uartFile)
    {
        printk(KERN_ERR "[INIT][CON] UART device is not open!\n");
        return -ENODEV;
    }

    oldfs = get_fs();
    set_fs(KERNEL_DS);  // Change address limit for kernel space write

    ret = kernel_write(Process.uartFile, buf, len, Process.uartFile->f_pos);

    set_fs(oldfs);  // Restore address limit

    if (ret < 0)
    {
        printk(KERN_ERR "[CTRL][CON] UART write failed: %ld\n", (long)ret);
    }

    return ret;
}

static int consoleThread(void *data)
{
    showThreadDiagnostics(Process.threadName);

    /* TODO :: To be considered later */
    // int len;
    // struct timespec64 ts;
    // static char count = 0;
    // char message[128];

    while (!kthread_should_stop())
    {
        /* TODO :: To be considered later */
        // memset(message, 0, sizeof(message));
        // ktime_get_real_ts64(&ts);
        // len = snprintf(message, sizeof(message), "---==[ %lld.%09ld :: 0x%x ]==---\r\n", (long long)ts.tv_sec, ts.tv_nsec, ++count);

        // len = snprintf(message, sizeof(message), "C\n");

        // uart_write(message, len);

        /**
         *
         * Reduce consumption of CPU resources
         * Add a short delay to prevent
         * busy waiting
         *
         */
        msleep(10); /* Release 90% of CPU resources */
    }

    return 0;
}

void consoleInit(void)
{
    int ret;

    ret = uart_open();

    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][CON] Cannot open UART device\n");
    }

    Process.threadHandle = kthread_create(consoleThread, NULL, Process.threadName);

    if (IS_ERR(Process.threadHandle))
    {
        printk(KERN_ERR "[INIT][CON] Failed to create kernel thread. Error code: %ld\n", PTR_ERR(Process.threadHandle));
    }
    else
    {
        printk(KERN_INFO "[INIT][CON] Created kthread for consoleThread\n");
        wake_up_process(Process.threadHandle);
    }
}

void consoleDestroy(void)
{
    uart_close();

    if (Process.threadHandle)
    {
        kthread_stop(Process.threadHandle);
        Process.threadHandle = NULL;
    }
    printk(KERN_INFO "[DESTROY][CON] Destroy consoleThread\n");
}
