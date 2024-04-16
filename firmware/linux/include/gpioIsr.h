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
#define GPIO_FPGA_INTERRUPT 50    // P9_14

int gpioKernelIsrInit(void);
void gpioKernelIsrDestroy(void);

#endif // GPIO_ISR_H