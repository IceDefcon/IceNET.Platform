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
//      [ISR] Work      //
//                      //
//                      //
//                      //
//////////////////////////

static irqreturn_t transferFpgaOutputISR(int irq, void *data)
{
    static int counter = 0;

    printk(KERN_INFO "[CTRL][ISR] Interrupt No[%d] received from FPGA @ Pin [%d]\n", counter, GPIO_FPGA_INTERRUPT);
    counter++;

    /* QUEUE :: Execution of transferFpgaOutput */
    queue_work(get_transferFpgaOutput_wq(), get_transferFpgaOutput_work());

    return IRQ_HANDLED;
}

static int interruptFromKernelInit(void)
{
    int result;

    result = gpio_request(GPIO_KERNEL_INTERRUPT, "   Response");
    if (result < 0) 
    {
        printk(KERN_ERR "[INIT][ISR] Failed GPIO Request :: Pin [%d]\n", GPIO_KERNEL_INTERRUPT);
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Request\n", GPIO_KERNEL_INTERRUPT);
    }

    // Set GPIO pin as an output
    result = gpio_direction_output(GPIO_KERNEL_INTERRUPT, 0); // Write low (0) to sink current to ground
    if (result < 0) 
    {
        printk(KERN_ERR "[INIT][ISR] Failed to set GPIO direction :: Pin [%d]\n", GPIO_KERNEL_INTERRUPT);
        gpio_free(GPIO_KERNEL_INTERRUPT);
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] as output\n", GPIO_KERNEL_INTERRUPT);
    }

    return result;
}

static int transferFpgaOutputInit(void)
{

    int irq_kernel;
    int result;

    result = gpio_request(GPIO_FPGA_INTERRUPT, "   Request");
    if (result < 0) 
    {
        printk(KERN_ERR "[INIT][ISR] Failed GPIO Request :: Pin [%d]\n", GPIO_FPGA_INTERRUPT);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Request\n", GPIO_FPGA_INTERRUPT);
    }

    result = gpio_direction_input(GPIO_FPGA_INTERRUPT);
    if (result < 0) 
    {
        printk(KERN_ERR "[INIT][ISR] Failed to set GPIO direction :: Pin [%d]\n", GPIO_FPGA_INTERRUPT);
        gpio_free(GPIO_FPGA_INTERRUPT);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] as input\n", GPIO_FPGA_INTERRUPT);
    }


    irq_kernel = gpio_to_irq(GPIO_FPGA_INTERRUPT);
    if (irq_kernel < 0) 
    {
        printk(KERN_ERR "[INIT][ISR] Failed to get IRQ number :: Pin [%d]\n", GPIO_FPGA_INTERRUPT);
        gpio_free(GPIO_FPGA_INTERRUPT);
        return irq_kernel;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] as interrupt\n", GPIO_FPGA_INTERRUPT);
    }

    result = request_irq(irq_kernel, transferFpgaOutputISR, IRQF_TRIGGER_RISING, "Request IRQ", NULL);
    if (result < 0) 
    {
        printk(KERN_ERR "[INIT][ISR] Failed to request IRQ number :: Pin [%d]\n", GPIO_FPGA_INTERRUPT);
        gpio_free(GPIO_FPGA_INTERRUPT);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Register transferFpgaOutputISR callback at Pin [%d] IRQ\n", GPIO_FPGA_INTERRUPT);
    }

    return 0;
}

static void transferFpgaOutputDestroy(void)
{
    int irq_kernel;

    irq_kernel = gpio_to_irq(GPIO_FPGA_INTERRUPT);
    free_irq(irq_kernel, NULL);
    gpio_free(GPIO_FPGA_INTERRUPT);
}

static void interruptFromKernelDestroy(void)
{
    gpio_free(GPIO_KERNEL_INTERRUPT);
}

void isrGpioInit(void)
{
    (void)interruptFromKernelInit();
    (void)transferFpgaOutputInit();
}

void isrGpioDestroy(void)
{
    transferFpgaOutputDestroy();
    interruptFromKernelDestroy();
    printk(KERN_INFO "[DESTROY][ISR] Destroy IRQ for GPIO Pins\n");
}
