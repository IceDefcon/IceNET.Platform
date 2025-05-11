/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#include "irqCtrl.h"
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

/* ISR */ static irqreturn_t InterruptFromFPGA_TimerISR(int irq, void *data);
/* ISR */ static irqreturn_t InterruptFromFPGA_WatchdogISR(int irq, void *data);
/* ISR */ static irqreturn_t InterruptFromFPGA_SpiISR(int irq, void *data);

static gpioInputProcessType gpioInputProcess[GPIO_IN_AMOUNT] =
{
    [GPIO_IN_ACTIVATE_SECONDARY_DMA] =
    {
        .irqNumber = 0,
        .gpioName = "SpiGPIO",
        .irqName = "Activate SPI.1",
        .isrFunctionName = "InterruptFromFPGA_SpiISR",
        .flags = IRQF_TRIGGER_RISING,
        .irqFlags = 0,
        .gpioNumber = GPIO_NUMBER_ACTIVATE_SECONDARY_DMA,
        .isrFunction = InterruptFromFPGA_SpiISR,
        .tryLock = 0,
    },

    [GPIO_IN_SCHEDULER_TIMER] =
    {
        .irqNumber = 0,
        .gpioName = "TimerGPIO",
        .irqName = "Scheduler Pulse",
        .isrFunctionName = "InterruptFromFPGA_TimerISR",
        .flags = IRQF_TRIGGER_RISING,
        .irqFlags = 0,
        .gpioNumber = GPIO_NUMBER_SCHEDULER_TIMER,
        .isrFunction = InterruptFromFPGA_TimerISR,
        .tryLock = 0,
    },

    [GPIO_IN_WATCHDOG_TICK] =
    {
        .irqNumber = 0,
        .gpioName = "WatchdogGPIO",
        .irqName = "Watchdog Tick",
        .isrFunctionName = "InterruptFromFPGA_WatchdogISR",
        .flags = IRQF_TRIGGER_RISING,
        .irqFlags = 0,
        .gpioNumber = GPIO_NUMBER_WATCHDOG_TICK,
        .isrFunction = InterruptFromFPGA_WatchdogISR,
        .tryLock = 0,
    },
};

static gpioOutputProcessType gpioOutputProcess[GPIO_OUT_AMOUNT] =
{
    [GPIO_OUT_RESET_FPGA] =
    {
        .gpioName = "FpgaResetGPIO",
        .gpioNumber = GPIO_NUMBER_RESET_FPGA,
    }
};

/* ISR */ static irqreturn_t InterruptFromFPGA_TimerISR(int irq, void *data)
{
    if(isShedulerReady())
    {
        setScheduler(SCH_MAIN_20MS);
        queue_work(get_scheduleTimer_wq(), get_scheduleTimer_work());
    }
    else
    {
        queue_work(get_scheduleTimer_wq(), get_scheduleTimer_work());
    }

    return IRQ_HANDLED;
}

/* ISR */ static irqreturn_t InterruptFromFPGA_WatchdogISR(int irq, void *data)
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

/* ISR */ static irqreturn_t InterruptFromFPGA_SpiISR(int irq, void *data)
{
#if 0
    static int counter = 0;

    printk(KERN_INFO "[CTRL][ISR] Interrupt No[%d] received from FPGA @ Pin [%d] :: TODO Checksum comparsion\n", counter, GPIO_NUMBER_ACTIVATE_SECONDARY_DMA);
    counter++;
#endif

    /* LOCK */ spin_lock_irqsave(&gpioInputProcess[GPIO_IN_ACTIVATE_SECONDARY_DMA].isrLock, gpioInputProcess[GPIO_IN_ACTIVATE_SECONDARY_DMA].irqFlags);


    /* CHECK */ if(!gpioInputProcess[GPIO_IN_ACTIVATE_SECONDARY_DMA].tryLock)
    {
        gpioInputProcess[GPIO_IN_ACTIVATE_SECONDARY_DMA].tryLock = 1;

        /* QUEUE :: Execution of masterTransferSecondary */
        queue_work(get_masterTransferSecondary_wq(), get_masterTransferSecondary_work());
    }
    else
    {
        printk(KERN_INFO "[CTRL][TASK] Spinlock is already held, skipping\n");
    }

    /* UNLOCK */ spin_unlock_irqrestore(&gpioInputProcess[GPIO_IN_ACTIVATE_SECONDARY_DMA].isrLock, gpioInputProcess[GPIO_IN_ACTIVATE_SECONDARY_DMA].irqFlags);

    return IRQ_HANDLED;
}

/* CLEAR */ void clearIsrLock(void)
{
    gpioInputProcess[GPIO_IN_ACTIVATE_SECONDARY_DMA].tryLock = 0;
}

static int initializeInterruptFromCPU(outputGpioType outGpio)
{
    int ret;

    ret = gpio_request(gpioOutputProcess[GPIO_OUT_RESET_FPGA].gpioNumber, gpioOutputProcess[GPIO_OUT_RESET_FPGA].gpioName);
    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed GPIO Request :: Pin [%d]\n", gpioOutputProcess[GPIO_OUT_RESET_FPGA].gpioNumber);
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Request\n", gpioOutputProcess[GPIO_OUT_RESET_FPGA].gpioNumber);
    }

    ret = gpio_direction_output(gpioOutputProcess[GPIO_OUT_RESET_FPGA].gpioNumber, 0);
    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to set GPIO direction :: Pin [%d]\n", gpioOutputProcess[GPIO_OUT_RESET_FPGA].gpioNumber);
        gpio_free(gpioOutputProcess[GPIO_OUT_RESET_FPGA].gpioNumber);
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Output\n", gpioOutputProcess[GPIO_OUT_RESET_FPGA].gpioNumber);
    }

    return ret;
}

static void destroyInterruptFromCPU(outputGpioType outGpio)
{
    gpio_free(gpioOutputProcess[outGpio].gpioNumber);
    printk(KERN_INFO "[DESTROY][ISR] Destroy GPIO Pin [%d] From CPU\n", gpioOutputProcess[outGpio].gpioNumber);
}

static int initializeInterruptFromFpga(inputGpioType inputGpio)
{
    int ret;

    ret = gpio_request(gpioInputProcess[inputGpio].gpioNumber, gpioInputProcess[inputGpio].gpioName);
    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed GPIO Request :: Pin [%d]\n", gpioInputProcess[inputGpio].gpioNumber);
        return ret;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Request\n", gpioInputProcess[inputGpio].gpioNumber);
    }

    ret = gpio_direction_input(gpioInputProcess[inputGpio].gpioNumber);
    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to set GPIO direction :: Pin [%d]\n", gpioInputProcess[inputGpio].gpioNumber);
        gpio_free(gpioInputProcess[inputGpio].gpioNumber);
        return ret;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Input\n", gpioInputProcess[inputGpio].gpioNumber);
    }

    gpio_set_value(gpioInputProcess[inputGpio].gpioNumber, 0);

    gpioInputProcess[inputGpio].irqNumber = gpio_to_irq(gpioInputProcess[inputGpio].gpioNumber);
    if (gpioInputProcess[inputGpio].irqNumber < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to get IRQ number :: Pin [%d]\n", gpioInputProcess[inputGpio].gpioNumber);
        gpio_free(gpioInputProcess[inputGpio].gpioNumber);
        return gpioInputProcess[inputGpio].irqNumber;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Interrupt\n", gpioInputProcess[inputGpio].gpioNumber);
    }

    ret = request_irq(gpioInputProcess[inputGpio].irqNumber, gpioInputProcess[inputGpio].isrFunction, gpioInputProcess[inputGpio].flags, gpioInputProcess[inputGpio].irqName, NULL);
    if (ret < 0)
    {
        printk(KERN_ERR "[INIT][ISR] Failed to request IRQ number :: Pin [%d]\n", gpioInputProcess[inputGpio].gpioNumber);
        gpio_free(gpioInputProcess[inputGpio].gpioNumber);
        return ret;
    }
    else
    {
        printk(KERN_ERR "[INIT][ISR] Setup GPIO Pin [%d] Register %s\n", gpioInputProcess[inputGpio].gpioNumber, gpioInputProcess[inputGpio].isrFunctionName);
    }

    spin_lock_init(&gpioInputProcess[inputGpio].isrLock);

    return ret;
}

static void destroyInterruptFromFPGA(inputGpioType inputGpio)
{
    free_irq(gpioInputProcess[inputGpio].irqNumber, NULL);
    gpio_free(gpioInputProcess[inputGpio].gpioNumber);
    printk(KERN_INFO "[DESTROY][ISR] Destroy GPIO Pin [%d] From FPGA\n", gpioInputProcess[inputGpio].gpioNumber);
}

void isrGpioInit(void)
{
    (void)initializeInterruptFromFpga(GPIO_IN_ACTIVATE_SECONDARY_DMA);
    (void)initializeInterruptFromFpga(GPIO_IN_SCHEDULER_TIMER);
    (void)initializeInterruptFromFpga(GPIO_IN_WATCHDOG_TICK);
    (void)initializeInterruptFromCPU(GPIO_OUT_RESET_FPGA);
}

void isrGpioDestroy(void)
{
    destroyInterruptFromCPU(GPIO_OUT_RESET_FPGA);
    destroyInterruptFromFPGA(GPIO_IN_ACTIVATE_SECONDARY_DMA);
    destroyInterruptFromFPGA(GPIO_IN_SCHEDULER_TIMER);
    destroyInterruptFromFPGA(GPIO_IN_WATCHDOG_TICK);

    printk(KERN_INFO "[DESTROY][ISR] All GPIO Pins Destroyed\n");
}
