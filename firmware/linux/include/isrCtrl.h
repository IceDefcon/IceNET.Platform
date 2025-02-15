/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
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

#define GPIO_TIMER_INTERRUPT_FROM_FPGA 115  // IN  :: PIN_A13 :: P9_27 :: TIMER_INT_FROM_FPGA
#define GPIO_SPI_INTERRUPT_FROM_CPU 48      // OUT :: PIN_A5  :: P9_15 :: INT_FROM_CPU
#define GPIO_SPI_INTERRUPT_FROM_FPGA 49     // IN  :: PIN_A9  :: P9_23 :: INT_FROM_FPGA
#define GPIO_WATCHDOG_INTERRUPT_FROM_FPGA 20// IN  :: PIN_A20 :: P9_41 :: WDG_INT_FROM_FPGA
#define GPIO_WATCHDOG_INTERRUPT_FROM_CPU 60 // OUT :: PIN_B4  :: P9_12 :: WDG_INT_FROM_CPU

void isrGpioInit(void);
void isrGpioDestroy(void);

#endif // GPIO_ISR_H
