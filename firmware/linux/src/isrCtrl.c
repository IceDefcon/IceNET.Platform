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

#include "isrCtrl.h"
#include "spiWork.h"

//////////////////////////
//                      //
//                      //
//                      //
//    [GPIO :: ISR]     //
//                      //
//                      //
//                      //
//////////////////////////

static irqreturn_t isrInterruptFromFpga(int irq, void *data)
{
    static int counter = 0;

    printk(KERN_INFO "[CTRL][ISR] Interrupt from FPGA [%d] @ Pin [%d]\n", counter, GPIO_KERNEL_INTERRUPT);
    counter++;

    queue_work(get_interruptFromFpga_wq(), get_interruptFromFpga_work());

    return IRQ_HANDLED;
}

static int isrInterruptFromKernelInit(void)
{
    int result;

    result = gpio_request(GPIO_FPGA_INTERRUPT, "   Response");
    if (result < 0) {
        printk(KERN_ERR "[INIT][GIO] Failed GPIO Request :: Pin [%d]\n", GPIO_FPGA_INTERRUPT);
        return result;
    }

    // Set GPIO pin as an output
    result = gpio_direction_output(GPIO_FPGA_INTERRUPT, 0); // Write low (0) to sink current to ground
    if (result < 0) {
        printk(KERN_ERR "[INIT][GIO] Failed to set GPIO direction :: Pin [%d]\n", GPIO_FPGA_INTERRUPT);
        gpio_free(GPIO_FPGA_INTERRUPT);
        return result;
    }
}

static int isrInterruptFromFpgaInit(void)
{

    int irq_kernel;
    int result;

    result = gpio_request(GPIO_KERNEL_INTERRUPT, "   Request");
    if (result < 0) 
    {
        printk(KERN_ERR "[INIT][ISR] Failed GPIO Request :: Pin [%d]\n", GPIO_KERNEL_INTERRUPT);
        return result;
    }
    result = gpio_direction_input(GPIO_KERNEL_INTERRUPT);
    if (result < 0) 
    {
        printk(KERN_ERR "[INIT][ISR] Failed to set GPIO direction :: Pin [%d]\n", GPIO_KERNEL_INTERRUPT);
        gpio_free(GPIO_KERNEL_INTERRUPT);
        return result;
    }
    irq_kernel = gpio_to_irq(GPIO_KERNEL_INTERRUPT);
    if (irq_kernel < 0) 
    {
        printk(KERN_ERR "[INIT][ISR] Failed to get IRQ number :: Pin [%d]\n", GPIO_KERNEL_INTERRUPT);
        gpio_free(GPIO_KERNEL_INTERRUPT);
        return irq_kernel;
    }
    result = request_irq(irq_kernel, isrInterruptFromFpga, IRQF_TRIGGER_RISING, "Request IRQ", NULL);
    if (result < 0) 
    {
        printk(KERN_ERR "[INIT][ISR] Failed to request IRQ number :: Pin [%d]\n", GPIO_KERNEL_INTERRUPT);
        gpio_free(GPIO_KERNEL_INTERRUPT);
        // spiDestroy(); TODO :: Do I need you here?
        return result;
    }
    printk(KERN_INFO "[INIT][ISR] Initialized\n");

    return 0;
}

static void isrInterruptFromFpgaDestroy(void)
{
    int irq_kernel;

    irq_kernel = gpio_to_irq(GPIO_KERNEL_INTERRUPT);
    free_irq(irq_kernel, NULL);
    gpio_free(GPIO_KERNEL_INTERRUPT);
    printk(KERN_INFO "[EXIT][ISR] Exit\n");
}

static void isrInterruptFromKernelDestroy(void)
{
    gpio_free(GPIO_FPGA_INTERRUPT);
}

void isrSetGpio(unsigned int gpio, int value)
{
    gpio_set_value(gpio, value);
}

void isrGpioInit(void)
{
    (void)isrInterruptFromKernelInit();
    (void)isrInterruptFromFpgaInit();
}

void isrGpioDestroy(void)
{
    isrInterruptFromFpgaDestroy();
    isrInterruptFromKernelDestroy();
}
