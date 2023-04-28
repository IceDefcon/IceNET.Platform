
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
static irq_handler_t  gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

static int __init gpio_init(void)
{
   int result = 0;
   printk(KERN_INFO "GPIO_TEST: Initializing the GPIO_TEST LKM\n");

   // Is the GPIO a valid GPIO number (e.g., the BBB has 4x32 but not all available)
   if (!gpio_is_valid(gpioLED))
   {
      printk(KERN_INFO "GPIO_TEST: invalid LED GPIO\n");
      return -ENODEV;
   }

   // Going to set up the LED. It is a GPIO in output mode and will be on by default
   ledOn = true;
   gpio_request(gpioLED, "sysfs");          // gpioLED is hardcoded to 49, request it
   gpio_direction_output(gpioLED, ledOn);   // Set the gpio to be in output mode and on
   
   // gpio_set_value(gpioLED, ledOn);       // Not required as set by line above (here for reference)
   gpio_export(gpioLED, false);             // Causes gpio49 to appear in /sys/class/gpio
   // the bool argument prevents the direction from being changed
   
   gpio_request(gpioButton, "sysfs");       // Set up the gpioButton
   gpio_direction_input(gpioButton);        // Set the button GPIO to be an input
   gpio_set_debounce(gpioButton, 200);      // Debounce the button with a delay of 200ms
   gpio_export(gpioButton, false);          // Causes gpio115 to appear in /sys/class/gpio
   // the bool argument prevents the direction from being changed
   
   // Perform a quick test to see that the button is working as expected on LKM load
   printk(KERN_INFO "GPIO_TEST: The button state is currently: %d\n", gpio_get_value(gpioButton));

   // GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
   irqNumber = gpio_to_irq(gpioButton);
   printk(KERN_INFO "GPIO_TEST: The button is mapped to IRQ: %d\n", irqNumber);

   // This next call requests an interrupt line
   result = request_irq(irqNumber,                       // The interrupt number requested
                  (irq_handler_t) gpio_irq_handler,      // The pointer to the handler function below
                  IRQF_TRIGGER_RISING,                   // Interrupt on rising edge (button press, not release)
                  "ebb_gpio_handler",                    // Used in /proc/interrupts to identify the owner
                  NULL);                                 // The *dev_id for shared interrupt lines, NULL is okay

   printk(KERN_INFO "GPIO_TEST: The interrupt request result is: %d\n", result);
   return result;
}

static void __exit gpio_exit(void)
{
   printk(KERN_INFO "GPIO_TEST: The button state is currently: %d\n", gpio_get_value(gpioButton));
   printk(KERN_INFO "GPIO_TEST: The button was pressed %d times\n", numberPresses);
   gpio_set_value(gpioLED, 0);              // Turn the LED off, makes it clear the device was unloaded
   gpio_unexport(gpioLED);                  // Unexport the LED GPIO
   free_irq(irqNumber, NULL);               // Free the IRQ number, no *dev_id required in this case
   gpio_unexport(gpioButton);               // Unexport the Button GPIO
   gpio_free(gpioLED);                      // Free the LED GPIO
   gpio_free(gpioButton);                   // Free the Button GPIO
   printk(KERN_INFO "GPIO_TEST: Goodbye from the LKM!\n");
}

static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
   ledOn = !ledOn;                          // Invert the LED state on each button press
   gpio_set_value(gpioLED, ledOn);          // Set the physical LED accordingly
   printk(KERN_INFO "GPIO_TEST: Interrupt! (button state is %d)\n", gpio_get_value(gpioButton));
   numberPresses++;                         // Global counter, will be outputted when the module is unloaded
   return (irq_handler_t) IRQ_HANDLED;      // Announce that the IRQ has been handled correctly
}

module_init(gpio_init);
module_exit(gpio_exit);