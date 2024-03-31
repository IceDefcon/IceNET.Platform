/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#ifndef GPIO_ISR_H
#define GPIO_ISR_H

//////////////////////////
//                      //
//                      //
//                      //
//    [GPIO :: ISR]     //
//                      //
//                      //
//                      //
//////////////////////////

#define GPIO_KERNEL_INTERRUPT 60    // P9_12

int gpioKernelIsrInit(void);

#endif // GPIO_ISR_H