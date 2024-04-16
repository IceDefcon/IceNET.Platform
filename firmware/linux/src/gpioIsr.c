/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#include "gpioIsr.h"
#include "workLoad.h"

//////////////////////////
//                      //
//                      //
//                      //
//    [GPIO :: ISR]     //
//                      //
//                      //
//                      //
//////////////////////////

static irqreturn_t gpioKernelIsr(int irq, void *data)
{
    static int counter = 0;

    printk(KERN_INFO "[FPGA][ISR] Kernel interrupt [%d] @ Pin [%d]\n", counter, GPIO_KERNEL_INTERRUPT);
    counter++;

    queue_work(get_kernel_wq(), get_kernel_work());

    return IRQ_HANDLED;
}

int gpioFpgaIsrInit(void)
{
    int result;

    result = gpio_request(GPIO_FPGA_INTERRUPT, "   Response");
    if (result < 0) {
        printk(KERN_ERR "Failed GPIO Request :: Pin [%d]\n", GPIO_FPGA_INTERRUPT);
        return result;
    }

    // Set GPIO pin as an output
    result = gpio_direction_output(GPIO_PIN, 0); // Write low (0) to sink current to ground
    if (result < 0) {
        printk(KERN_ERR "Failed to set GPIO direction :: Pin [%d]\n", GPIO_PIN);
        gpio_free(GPIO_PIN);
        return result;
    }
}

// Function to set GPIO pin to high (off)
void set_gpio_high(void)
{
    gpio_set_value(GPIO_FPGA_INTERRUPT, 1); // Set GPIO pin to high
}

// Function to set GPIO pin to low (on)
void set_gpio_low(void)
{
    gpio_set_value(GPIO_FPGA_INTERRUPT, 0); // Set GPIO pin to low
}

int gpioKernelIsrInit(void)
{

    int irq_kernel;
    int result;

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
    result = request_irq(irq_kernel, gpioKernelIsr, IRQF_TRIGGER_RISING, "Request IRQ", NULL);
    if (result < 0) 
    {
        printk(KERN_ERR "[FPGA][IRQ] Failed to request IRQ number :: Pin [%d]\n", GPIO_KERNEL_INTERRUPT);
        gpio_free(GPIO_KERNEL_INTERRUPT);
        // spiDestroy(); TODO :: Do I need you here?
        return result;
    }
    printk(KERN_INFO "[FPGA][IRQ] Initialized\n");

    return 0;
}

void gpioKernelIsrDestroy(void)
{
    int irq_kernel;

    irq_kernel = gpio_to_irq(GPIO_KERNEL_INTERRUPT);
    free_irq(irq_kernel, NULL);
    gpio_free(GPIO_KERNEL_INTERRUPT);
     printk(KERN_INFO "[FPGA][IRQ] Exit\n");
}
