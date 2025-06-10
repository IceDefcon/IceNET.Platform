/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#ifndef TASK_H
#define TASK_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

typedef enum
{
    TASK_SECONDARY_SPI,
    TASK_AMOUNT,
} taskletEnum;

typedef struct
{
    int irqNumber;
    char *irqName;
    unsigned long flags;
    struct tasklet_struct tasklet;
    irqreturn_t (*isrFunction)(int, void *);
    spinlock_t isrLock;
    int tryLock;
} taskletProcessType;

/* INIT*/ int taskletInit(void);
/* EXIT */ void taskletDestroy(void);
/* UNLOCK */ void unlockTaskletMutex(void);

#endif // TASK_H
