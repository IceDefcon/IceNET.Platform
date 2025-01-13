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
#define GPIO_WATCHDOG_INTERRUPT_FROM_FPGA 20  // IN  :: PIN_A20 :: P9_41 :: WDG_INT_FROM_FPGA
#define GPIO_WATCHDOG_INTERRUPT_FROM_CPU 60  // OUT  :: PIN_B4  :: P9_12 :: WDG_INT_FROM_CPU

void isrGpioInit(void);
void isrGpioDestroy(void);

#endif // GPIO_ISR_H
