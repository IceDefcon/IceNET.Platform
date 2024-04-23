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

#define GPIO_KERNEL_INTERRUPT 60 // IN :: P9_12
#define GPIO_FPGA_INTERRUPT 50 // OUT :: P9_14

void isrSetGpio(unsigned int gpio, int value);
void isrGpioInit(void);
void isrGpioDestroy(void);

#endif // GPIO_ISR_H