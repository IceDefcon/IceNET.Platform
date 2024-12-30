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

#define GPIO_INTERRUPT_FROM_CPU 48  // OUT :: P9_15 :: PIN_A5 :: INT_FROM_CPU
#define GPIO_INTERRUPT_FROM_FPGA 49 // IN  :: P9_23 :: PIN_A9 :: INT_FROM_FPGA
#define GPIO_WATCHDOG_INTERRUPT 20  // IN  :: P9_41 :: PIN_A20 :: WATCHDOG_INTERRUPT

void isrGpioInit(void);
void isrGpioDestroy(void);

#endif // GPIO_ISR_H
