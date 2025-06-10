/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "spiDmaCtrl.h"
#include "charDevice.h"
#include "irqCtrl.h"
#include "task.h"

/* HANDLER */ static irqreturn_t spiSecondaryHandler(int irq, void *dev_id);
/* TASK */ void masterTransferSecondarytask(unsigned long data);

static taskletProcessType taskletProcess[TASK_AMOUNT] =
{
    [TASK_SECONDARY_SPI] =
    {
        .irqNumber = 0,
        .irqName = "gpioIrqSpi",
        .flags = IRQF_TRIGGER_RISING,
        .tasklet = {0},
        .isrFunction = spiSecondaryHandler,
        .tryLock = 0,
    }
};

/* UNLOCK */
void unlockTaskletLock(void)
{
    spin_unlock(&taskletProcess[TASK_SECONDARY_SPI].isrLock);
}

static irqreturn_t spiSecondaryHandler(int irq, void *dev_id)
{
    unsigned long flags;

    /* Try to acquire spinlock */
    spin_lock_irqsave(&taskletProcess[TASK_SECONDARY_SPI].isrLock, flags);

    if (!taskletProcess[TASK_SECONDARY_SPI].tryLock)
    {
        taskletProcess[TASK_SECONDARY_SPI].tryLock = 1;
        /* QUEUE :: Execution of masterTransferSecondary */
        tasklet_schedule(&taskletProcess[TASK_SECONDARY_SPI].tasklet);
    }
    else
    {
        printk(KERN_INFO "[CTRL][TASK] Spinlock is already held, skipping\n");
    }

    /* Unlock spinlock */
    spin_unlock_irqrestore(&taskletProcess[TASK_SECONDARY_SPI].isrLock, flags);

    return IRQ_HANDLED;
}

/* INIT*/
int taskletInit(void)
{
    int ret;

    if (!gpio_is_valid(GPIO_NUMBER_ACTIVATE_SECONDARY_DMA))
    {
        pr_err("[ERNO] [TASK] Invalid GPIO\n");
        return -ENODEV;
    }

    ret = gpio_request(GPIO_NUMBER_ACTIVATE_SECONDARY_DMA, "sysfs");
    if (ret)
    {
        pr_err("[ERNO] [TASK] GPIO request failed\n");
        return ret;
    }

    gpio_direction_input(GPIO_NUMBER_ACTIVATE_SECONDARY_DMA);

    taskletProcess[TASK_SECONDARY_SPI].irqNumber = gpio_to_irq(GPIO_NUMBER_ACTIVATE_SECONDARY_DMA);
    if (taskletProcess[TASK_SECONDARY_SPI].irqNumber < 0)
    {
        pr_err("[ERNO][ISR] Unable to get IRQ number for GPIO %d\n", GPIO_NUMBER_ACTIVATE_SECONDARY_DMA);
        gpio_free(GPIO_NUMBER_ACTIVATE_SECONDARY_DMA);
        return taskletProcess[TASK_SECONDARY_SPI].irqNumber;
    }

    ret = request_irq(taskletProcess[TASK_SECONDARY_SPI].irqNumber, taskletProcess[TASK_SECONDARY_SPI].isrFunction, taskletProcess[TASK_SECONDARY_SPI].flags, taskletProcess[TASK_SECONDARY_SPI].irqName, NULL);
    if (ret)
    {
        pr_err("Unable to request IRQ\n");
        gpio_free(GPIO_NUMBER_ACTIVATE_SECONDARY_DMA);
        return ret;
    }

    tasklet_init(&taskletProcess[TASK_SECONDARY_SPI].tasklet, masterTransferSecondarytask, 0);
    spin_lock_init(&taskletProcess[TASK_SECONDARY_SPI].isrLock);

    pr_info("[INIT] [TASK] Tasklet configured\n");

    return 0;
}

/* EXIT */ void taskletDestroy(void)
{
    free_irq(taskletProcess[TASK_SECONDARY_SPI].irqNumber, NULL);
    gpio_free(GPIO_NUMBER_ACTIVATE_SECONDARY_DMA);

    tasklet_kill(&taskletProcess[TASK_SECONDARY_SPI].tasklet);

    pr_info("[DESTROY] [TASK] Tasklet Destroyed\n");
}
