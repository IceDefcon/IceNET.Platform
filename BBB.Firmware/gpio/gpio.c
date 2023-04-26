
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>                   // Required for the GPIO functions
#include <linux/interrupt.h>              // Required for the IRQ code

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marek Ice");
MODULE_DESCRIPTION("Kernel module :: GPIO :: Cortex-A8");
MODULE_VERSION("1.0.0");

static unsigned int gpioLED = 49;         // Hard coding the LED gpio for this example to P9_23 (GPIO49)
static unsigned int gpioButton = 115;     // Hard coding the button gpio for this example to P9_27 (GPIO115)
static unsigned int irqNumber;            // Used to share the IRQ number within this file
static unsigned int numberPresses = 0;    // For information, store the number of button presses
static bool	ledOn = 0;                    // Is the LED on or off? Used to invert its state (off by default)

// Function prototype for the custom IRQ handler function -- see below for the implementation
static irq_handler_t  ebbgpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

static int __init gpio_init(void)
{
   return 0
}

static void __exit gpio_exit(void)
{

}

module_init(gpio_init);
module_exit(gpio_exit);