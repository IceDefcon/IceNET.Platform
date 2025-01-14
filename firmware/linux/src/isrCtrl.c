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
#include "watchdog.h"

//////////////////////////
//                      //
//                      //
//                      //
//      [ISR] Work      //
//                      //
//                      //
//                      //
//////////////////////////

/* ISR */
static irqreturn_t watchdogISR(int irq, void *data)
{
    watchdogProcess* tmpProcess = watchdog_getProcess();

    watchdog_lockWatchdogMutex();
    if(tmpProcess->indicatorCurrent == 0xFA)
    {
        tmpProcess->indicatorCurrent = 0x00;
    }
    else
    {
        tmpProcess->indicatorCurrent++;
    }
    watchdog_unlockWatchdogMutex();

    gpio_set_value(GPIO_WATCHDOG_INTERRUPT_FROM_CPU, 1);
    gpio_set_value(GPIO_WATCHDOG_INTERRUPT_FROM_CPU, 0);

    return IRQ_HANDLED;
}

static irqreturn_t transferFpgaOutputISR(int irq, void *data)
{
    static int counter = 0;

    printk(KERN_INFO "[CTRL][ISR] Interrupt No[%d] received from FPGA @ Pin [%d]\n", counter, GPIO_INTERRUPT_FROM_FPGA);
    counter++;

    /* QUEUE :: Execution of transferFpgaOutput */
    queue_work(get_transferFpgaOutput_wq(), get_transferFpgaOutput_work());

    return IRQ_HANDLED;
}

/* INIT */
static int interruptWatchdogInit(void)
{

    int irq_kernel;
    int result;

    result = gpio_request(GPIO_WATCHDOG_INTERRUPT_FROM_FPGA, "   Request");
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed GPIO Request :: Pin [%d]\n", GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Request\n", GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
    }

    result = gpio_direction_input(GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to set GPIO direction :: Pin [%d]\n", GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
        gpio_free(GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Input\n", GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
    }


    irq_kernel = gpio_to_irq(GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
    if (irq_kernel < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to get IRQ number :: Pin [%d]\n", GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
        gpio_free(GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
        return irq_kernel;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Interrupt\n", GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
    }

    result = request_irq(irq_kernel, watchdogISR, IRQF_TRIGGER_RISING, "Request IRQ", NULL);
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to request IRQ number :: Pin [%d]\n", GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
        gpio_free(GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Register watchdogISR callback at Pin [%d] IRQ\n", GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
    }

    return 0;
}

static int interruptFromKernelInit(void)
{
    int result;

    result = gpio_request(GPIO_INTERRUPT_FROM_CPU, "   Response");
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed GPIO Request :: Pin [%d]\n", GPIO_INTERRUPT_FROM_CPU);
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Request\n", GPIO_INTERRUPT_FROM_CPU);
    }

    // Set GPIO pin as an output
    result = gpio_direction_output(GPIO_INTERRUPT_FROM_CPU, 0); // Write low (0) to sink current to ground
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to set GPIO direction :: Pin [%d]\n", GPIO_INTERRUPT_FROM_CPU);
        gpio_free(GPIO_INTERRUPT_FROM_CPU);
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Output\n", GPIO_INTERRUPT_FROM_CPU);
    }

    return result;
}

static int transferFpgaOutputInit(void)
{

    int irq_kernel;
    int result;

    result = gpio_request(GPIO_INTERRUPT_FROM_FPGA, "   Request");
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed GPIO Request :: Pin [%d]\n", GPIO_INTERRUPT_FROM_FPGA);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Request\n", GPIO_INTERRUPT_FROM_FPGA);
    }

    result = gpio_direction_input(GPIO_INTERRUPT_FROM_FPGA);
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to set GPIO direction :: Pin [%d]\n", GPIO_INTERRUPT_FROM_FPGA);
        gpio_free(GPIO_INTERRUPT_FROM_FPGA);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Input\n", GPIO_INTERRUPT_FROM_FPGA);
    }


    irq_kernel = gpio_to_irq(GPIO_INTERRUPT_FROM_FPGA);
    if (irq_kernel < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to get IRQ number :: Pin [%d]\n", GPIO_INTERRUPT_FROM_FPGA);
        gpio_free(GPIO_INTERRUPT_FROM_FPGA);
        return irq_kernel;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Interrupt\n", GPIO_INTERRUPT_FROM_FPGA);
    }

    result = request_irq(irq_kernel, transferFpgaOutputISR, IRQF_TRIGGER_RISING, "Request IRQ", NULL);
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to request IRQ number :: Pin [%d]\n", GPIO_INTERRUPT_FROM_FPGA);
        gpio_free(GPIO_INTERRUPT_FROM_FPGA);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Register transferFpgaOutputISR callback at Pin [%d] IRQ\n", GPIO_INTERRUPT_FROM_FPGA);
    }

    return 0;
}

static int watchdogInterruptFromKernelInit(void)
{
    int result;

    result = gpio_request(GPIO_WATCHDOG_INTERRUPT_FROM_CPU, "   Response");
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed GPIO Request :: Pin [%d]\n", GPIO_WATCHDOG_INTERRUPT_FROM_CPU);
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Request\n", GPIO_WATCHDOG_INTERRUPT_FROM_CPU);
    }

    // Set GPIO pin as an output
    result = gpio_direction_output(GPIO_WATCHDOG_INTERRUPT_FROM_CPU, 0); // Write low (0) to sink current to ground
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to set GPIO direction :: Pin [%d]\n", GPIO_WATCHDOG_INTERRUPT_FROM_CPU);
        gpio_free(GPIO_WATCHDOG_INTERRUPT_FROM_CPU);
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Output\n", GPIO_WATCHDOG_INTERRUPT_FROM_CPU);
    }

    return result;
}

/* DESTROY */
static void watchdogInterruptFromKernelDestroy(void)
{
    gpio_free(GPIO_INTERRUPT_FROM_CPU);
}

static void transferFpgaOutputDestroy(void)
{
    int irq_kernel;

    irq_kernel = gpio_to_irq(GPIO_INTERRUPT_FROM_FPGA);
    free_irq(irq_kernel, NULL);
    gpio_free(GPIO_INTERRUPT_FROM_FPGA);
}

static void interruptFromKernelDestroy(void)
{
    gpio_free(GPIO_INTERRUPT_FROM_CPU);
}

static void interruptWatchdogDestroy(void)
{
    int irq_kernel;

    irq_kernel = gpio_to_irq(GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
    free_irq(irq_kernel, NULL);
    gpio_free(GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
}

void isrGpioInit(void)
{
    (void)interruptWatchdogInit();
    (void)interruptFromKernelInit();
    (void)transferFpgaOutputInit();
    (void)watchdogInterruptFromKernelInit();
}

void isrGpioDestroy(void)
{
    watchdogInterruptFromKernelDestroy();
    transferFpgaOutputDestroy();
    interruptFromKernelDestroy();
    interruptWatchdogDestroy();
    printk(KERN_INFO "[DESTROY][ISR] Destroy IRQ for GPIO Pins\n");
}
