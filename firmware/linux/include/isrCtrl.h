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

#define GPIO_KERNEL_INTERRUPT 48 	// OUT :: P9_15 :: PIN_B20
#define GPIO_FPGA_INTERRUPT 49 		// IN  :: P9_23 ::
#define GPIO_WATCHDOG_INTERRUPT 20 	// IN  :: P9_41

void isrGpioInit(void);
void isrGpioDestroy(void);

#endif // GPIO_ISR_H
