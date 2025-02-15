/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#include "isrCtrl.h"
#include "spiWork.h"
#include "watchdog.h"
#include "scheduler.h"

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
static irqreturn_t InterruptFromFPGA_TimerISR(int irq, void *data)
{
#if 0 /* TODO :: Temporarily turned off */
    if(isShedulerReady())
    {
        setScheduler(SCH_MAIN);
    }
#endif
    return IRQ_HANDLED;
}

static irqreturn_t InterruptFromFPGA_WatchdogISR(int irq, void *data)
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

static irqreturn_t InterruptFromFPGA_SpiISR(int irq, void *data)
{
    static int counter = 0;

    printk(KERN_INFO "[CTRL][ISR] Interrupt No[%d] received from FPGA @ Pin [%d] :: TODO Checksum comparsion\n", counter, GPIO_SPI_INTERRUPT_FROM_FPGA);
    counter++;

    /* QUEUE :: Execution of masterTransferSecondary */
    queue_work(get_masterTransferSecondary_wq(), get_masterTransferSecondary_work());
    return IRQ_HANDLED;
}

/* INIT */
static int InterruptFromFPGA_TimerInit(void)
{

    int irq_kernel;
    int result;

    result = gpio_request(GPIO_TIMER_INTERRUPT_FROM_FPGA, "   Timer");
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed GPIO Request :: Pin [%d]\n", GPIO_TIMER_INTERRUPT_FROM_FPGA);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Request\n", GPIO_TIMER_INTERRUPT_FROM_FPGA);
    }

    result = gpio_direction_input(GPIO_TIMER_INTERRUPT_FROM_FPGA);
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to set GPIO direction :: Pin [%d]\n", GPIO_TIMER_INTERRUPT_FROM_FPGA);
        gpio_free(GPIO_TIMER_INTERRUPT_FROM_FPGA);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Input\n", GPIO_TIMER_INTERRUPT_FROM_FPGA);
    }

    irq_kernel = gpio_to_irq(GPIO_TIMER_INTERRUPT_FROM_FPGA);
    if (irq_kernel < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to get IRQ number :: Pin [%d]\n", GPIO_TIMER_INTERRUPT_FROM_FPGA);
        gpio_free(GPIO_TIMER_INTERRUPT_FROM_FPGA);
        return irq_kernel;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Interrupt\n", GPIO_TIMER_INTERRUPT_FROM_FPGA);
    }

    result = request_irq(irq_kernel, InterruptFromFPGA_TimerISR, IRQF_TRIGGER_RISING, "Timer IRQ", NULL);
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to request IRQ number :: Pin [%d]\n", GPIO_TIMER_INTERRUPT_FROM_FPGA);
        gpio_free(GPIO_TIMER_INTERRUPT_FROM_FPGA);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Register InterruptFromFPGA_TimerISR callback\n", GPIO_TIMER_INTERRUPT_FROM_FPGA);
    }

    return 0;
}

static int InterruptFromFPGA_WatchdogInit(void)
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

    result = request_irq(irq_kernel, InterruptFromFPGA_WatchdogISR, IRQF_TRIGGER_RISING, "Request IRQ", NULL);
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to request IRQ number :: Pin [%d]\n", GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
        gpio_free(GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Register InterruptFromFPGA_WatchdogISR callback\n", GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
    }

    return 0;
}

static int InterruptFromCPU_SpiInit(void)
{
    int result;

    result = gpio_request(GPIO_SPI_INTERRUPT_FROM_CPU, "   Response");
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed GPIO Request :: Pin [%d]\n", GPIO_SPI_INTERRUPT_FROM_CPU);
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Request\n", GPIO_SPI_INTERRUPT_FROM_CPU);
    }

    result = gpio_direction_output(GPIO_SPI_INTERRUPT_FROM_CPU, 0); // Write low (0) to sink current to ground
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to set GPIO direction :: Pin [%d]\n", GPIO_SPI_INTERRUPT_FROM_CPU);
        gpio_free(GPIO_SPI_INTERRUPT_FROM_CPU);
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Output\n", GPIO_SPI_INTERRUPT_FROM_CPU);
    }

    return result;
}

static int InterruptFromFPGA_SpiInit(void)
{

    int irq_kernel;
    int result;

    result = gpio_request(GPIO_SPI_INTERRUPT_FROM_FPGA, "   Request");
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed GPIO Request :: Pin [%d]\n", GPIO_SPI_INTERRUPT_FROM_FPGA);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Request\n", GPIO_SPI_INTERRUPT_FROM_FPGA);
    }

    result = gpio_direction_input(GPIO_SPI_INTERRUPT_FROM_FPGA);
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to set GPIO direction :: Pin [%d]\n", GPIO_SPI_INTERRUPT_FROM_FPGA);
        gpio_free(GPIO_SPI_INTERRUPT_FROM_FPGA);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Input\n", GPIO_SPI_INTERRUPT_FROM_FPGA);
    }


    irq_kernel = gpio_to_irq(GPIO_SPI_INTERRUPT_FROM_FPGA);
    if (irq_kernel < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to get IRQ number :: Pin [%d]\n", GPIO_SPI_INTERRUPT_FROM_FPGA);
        gpio_free(GPIO_SPI_INTERRUPT_FROM_FPGA);
        return irq_kernel;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Interrupt\n", GPIO_SPI_INTERRUPT_FROM_FPGA);
    }

    result = request_irq(irq_kernel, InterruptFromFPGA_SpiISR, IRQF_TRIGGER_RISING, "Request IRQ", NULL);
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to request IRQ number :: Pin [%d]\n", GPIO_SPI_INTERRUPT_FROM_FPGA);
        gpio_free(GPIO_SPI_INTERRUPT_FROM_FPGA);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Register InterruptFromFPGA_SpiISR callback\n", GPIO_SPI_INTERRUPT_FROM_FPGA);
    }

    return 0;
}

static int InterruptFromCPU_WatchdogInit(void)
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
static void InterruptFromCPU_WatchdogDestroy(void)
{
    gpio_free(GPIO_SPI_INTERRUPT_FROM_CPU);
}

static void InterruptFromFPGA_SpiDestroy(void)
{
    int irq_kernel;

    irq_kernel = gpio_to_irq(GPIO_SPI_INTERRUPT_FROM_FPGA);
    free_irq(irq_kernel, NULL);
    gpio_free(GPIO_SPI_INTERRUPT_FROM_FPGA);
}

static void InterruptFromCPU_SpiDestroy(void)
{
    gpio_free(GPIO_SPI_INTERRUPT_FROM_CPU);
}

static void InterruptFromFPGA_WatchdogDestroy(void)
{
    int irq_kernel;

    irq_kernel = gpio_to_irq(GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
    free_irq(irq_kernel, NULL);
    gpio_free(GPIO_WATCHDOG_INTERRUPT_FROM_FPGA);
}

static void InterruptFromFPGA_TimerDestroy(void)
{
    int irq_kernel;

    irq_kernel = gpio_to_irq(GPIO_TIMER_INTERRUPT_FROM_FPGA);
    free_irq(irq_kernel, NULL);
    gpio_free(GPIO_TIMER_INTERRUPT_FROM_FPGA);
}

void isrGpioInit(void)
{
    (void)InterruptFromFPGA_TimerInit();
    (void)InterruptFromFPGA_WatchdogInit();
    (void)InterruptFromFPGA_SpiInit();

    (void)InterruptFromCPU_SpiInit();
    (void)InterruptFromCPU_WatchdogInit();
}

void isrGpioDestroy(void)
{
    InterruptFromCPU_WatchdogDestroy();
    InterruptFromCPU_SpiDestroy();

    InterruptFromFPGA_SpiDestroy();
    InterruptFromFPGA_WatchdogDestroy();
    InterruptFromFPGA_TimerDestroy();

    printk(KERN_INFO "[DESTROY][ISR] Destroy IRQ for GPIO Pins\n");
}
