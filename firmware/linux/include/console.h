/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2024
 *
 */

#ifndef UART_CONSOLE_H
#define UART_CONSOLE_H

#include <linux/fs.h>        // for struct file
#include <linux/mutex.h>     // for struct mutex
#include <linux/kthread.h>   // for struct task_struct

//////////////////
//              //
//              //
//              //
// UART Console //
//              //
//              //
//              //
//////////////////

typedef struct
{
    struct file *uartFile;
    struct task_struct *threadHandle;
    struct mutex stateMutex;
} consoleProcess;

void consoleInit(void);
void consoleDestroy(void);

#endif // UART_CONSOLE_H
