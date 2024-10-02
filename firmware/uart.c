#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>

#define DEVICE "/dev/ttyS1"   // Define the UART device (use the correct tty for your setup)

static struct file *uart_filp;

static int uart_open(void)
{
    // Open the UART device
    uart_filp = filp_open(DEVICE, O_RDWR | O_NOCTTY | O_NDELAY, 0);
    if (IS_ERR(uart_filp)) {
        printk(KERN_ERR "Failed to open UART device: %ld\n", PTR_ERR(uart_filp));
        return PTR_ERR(uart_filp);
    }
    printk(KERN_INFO "UART device opened successfully.\n");
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

    if (ret < 0) {
        printk(KERN_ERR "UART write failed: %ld\n", ret);
    } else {
        printk(KERN_INFO "Sent to UART: %s\n", buf);
    }

    return ret;
}

static int __init uart_module_init(void)
{
    int ret;

    // Open the UART device
    ret = uart_open();
    if (ret < 0) {
        return ret;
    }

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
