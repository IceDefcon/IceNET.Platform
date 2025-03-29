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

    watchdog_spinLockCtrl(CTRL_LOCK);
    if(tmpProcess->indicatorCurrent == 0xFA)
    {
        tmpProcess->indicatorCurrent = 0x00;
    }
    else
    {
        tmpProcess->indicatorCurrent++;
    }
    watchdog_spinLockCtrl(CTRL_UNLOCK);

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

static int initializeInterruptFromCPU(outputGpioType outGpio)
{
    int result;
    uint32_t gpioNumber;

    switch(outGpio)
    {
        case GPIO_OUT_OFFLOAD_FIFO:
            gpioNumber = GPIO_SPI_INTERRUPT_FROM_CPU;
            break;

        case GPIO_OUT_PERIPHERALS_CONFIG_DONE:
            gpioNumber = GPIO_CONF_DONE_INTERRUPT_FROM_CPU;
            break;

        case GPIO_OUT_RESET_FPGA:
            gpioNumber = GPIO_RESET_FROM_CPU;
            break;

        default:
            printk(KERN_ERR "[DESTROY][ISR] Unknown type of the output GPIO\n");
            break;
    };

    result = gpio_request(gpioNumber, "SPI_INT_FROM_CPU");
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed GPIO Request :: Pin [%d]\n", gpioNumber);
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Request\n", gpioNumber);
    }

    result = gpio_direction_output(gpioNumber, 0); // Write low (0) to sink current to ground
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to set GPIO direction :: Pin [%d]\n", gpioNumber);
        gpio_free(gpioNumber);
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Output\n", gpioNumber);
    }

    return result;
}

static void destroyInterruptFromCPU(outputGpioType outGpio)
{
    uint32_t gpioNumber;

    switch(outGpio)
    {
        case GPIO_OUT_OFFLOAD_FIFO:
            gpioNumber = GPIO_SPI_INTERRUPT_FROM_CPU;
            break;

        case GPIO_OUT_PERIPHERALS_CONFIG_DONE:
            gpioNumber = GPIO_CONF_DONE_INTERRUPT_FROM_CPU;
            break;

        case GPIO_OUT_RESET_FPGA:
            gpioNumber = GPIO_RESET_FROM_CPU;
            break;

        default:
            printk(KERN_ERR "[DESTROY][ISR] Unknown type of the output GPIO\n");
            break;
    };

    gpio_free(gpioNumber);
    printk(KERN_INFO "[DESTROY][ISR] Destroy GPIO Pin [%d] From CPU\n", gpioNumber);
}

static int initializeInterruptFromFpga(inputGpioType inputGpio)
{
    int result;
    int irq_kernel;
    char* irqProcess;
    uint32_t gpioNumber;
    irqreturn_t (*isrFunction)(int, void *);

    switch(inputGpio)
    {
        case GPIO_IN_ACTIVATE_SECONDARY_DMA:
            gpioNumber = GPIO_SPI_INTERRUPT_FROM_FPGA;
            isrFunction = InterruptFromFPGA_SpiISR;
            irqProcess = "Activate SPI.1";
            break;

        case GPIO_IN_SCHEDULER_TIMER:
            gpioNumber = GPIO_TIMER_INTERRUPT_FROM_FPGA;
            isrFunction = InterruptFromFPGA_TimerISR;
            irqProcess = "Scheduler Pulse";
            break;

        case GPIO_IN_WATCHDOG_TICK:
            gpioNumber = GPIO_WATCHDOG_INTERRUPT_FROM_FPGA;
            isrFunction = InterruptFromFPGA_WatchdogISR;
            irqProcess = "Watchdog Tick";
            break;

        default:
            printk(KERN_ERR "[DESTROY][ISR] Unknown type of the input GPIO\n");
            break;
    };

    result = gpio_request(gpioNumber, "WDG_INT_FROM_FPGA");
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed GPIO Request :: Pin [%d]\n", gpioNumber);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Request\n", gpioNumber);
    }

    result = gpio_direction_input(gpioNumber);
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to set GPIO direction :: Pin [%d]\n", gpioNumber);
        gpio_free(gpioNumber);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Input\n", gpioNumber);
    }

    gpio_set_value(gpioNumber, 0);

    irq_kernel = gpio_to_irq(gpioNumber);
    if (irq_kernel < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to get IRQ number :: Pin [%d]\n", gpioNumber);
        gpio_free(gpioNumber);
        return irq_kernel;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Interrupt\n", gpioNumber);
    }

    result = request_irq(irq_kernel, isrFunction, IRQF_TRIGGER_RISING, irqProcess, NULL);
    if (result < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to request IRQ number :: Pin [%d]\n", gpioNumber);
        gpio_free(gpioNumber);
        return result;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Register callback\n", gpioNumber);
    }

    return result;
}

static void destroyInterruptFromFPGA(inputGpioType inputGpio)
{
    int irq_kernel;
    uint32_t gpioNumber;

    switch(inputGpio)
    {
        case GPIO_IN_ACTIVATE_SECONDARY_DMA:
            gpioNumber = GPIO_SPI_INTERRUPT_FROM_FPGA;
            break;

        case GPIO_IN_SCHEDULER_TIMER:
            gpioNumber = GPIO_TIMER_INTERRUPT_FROM_FPGA;
            break;

        case GPIO_IN_WATCHDOG_TICK:
            gpioNumber = GPIO_WATCHDOG_INTERRUPT_FROM_FPGA;
            break;

        default:
            printk(KERN_ERR "[DESTROY][ISR] Unknown type of the input GPIO\n");
            break;
    };

    irq_kernel = gpio_to_irq(gpioNumber);
    free_irq(irq_kernel, NULL);
    gpio_free(gpioNumber);
    printk(KERN_INFO "[DESTROY][ISR] Destroy GPIO Pin [%d] From FPGA\n", gpioNumber);
}

void isrGpioInit(void)
{
    (void)initializeInterruptFromFpga(GPIO_IN_ACTIVATE_SECONDARY_DMA);
    (void)initializeInterruptFromFpga(GPIO_IN_SCHEDULER_TIMER);
    (void)initializeInterruptFromFpga(GPIO_IN_WATCHDOG_TICK);

    (void)initializeInterruptFromCPU(GPIO_OUT_OFFLOAD_FIFO);
    (void)initializeInterruptFromCPU(GPIO_OUT_PERIPHERALS_CONFIG_DONE);
    (void)initializeInterruptFromCPU(GPIO_OUT_RESET_FPGA);
}

void isrGpioDestroy(void)
{
    destroyInterruptFromCPU(GPIO_OUT_OFFLOAD_FIFO);
    destroyInterruptFromCPU(GPIO_OUT_PERIPHERALS_CONFIG_DONE);
    destroyInterruptFromCPU(GPIO_OUT_RESET_FPGA);

    destroyInterruptFromFPGA(GPIO_IN_ACTIVATE_SECONDARY_DMA);
    destroyInterruptFromFPGA(GPIO_IN_SCHEDULER_TIMER);
    destroyInterruptFromFPGA(GPIO_IN_WATCHDOG_TICK);

    printk(KERN_INFO "[DESTROY][ISR] All GPIO Pins Destroyed\n");
}
