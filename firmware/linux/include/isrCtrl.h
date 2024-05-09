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

#define GPIO_KERNEL_INTERRUPT 50 // IN :: P9_14
#define GPIO_FPGA_INTERRUPT 60 // OUT :: P9_12

void isrSetGpio(unsigned int gpio, int value);
void isrGpioInit(void);
void isrGpioDestroy(void);

#endif // GPIO_ISR_H