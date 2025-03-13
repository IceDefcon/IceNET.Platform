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

#define GPIO_SPI_INTERRUPT_FROM_CPU 194         // HEADER_PIN_15 :: GPIO12
#define GPIO_SPI_INTERRUPT_FROM_FPGA 149        // HEADER_PIN_29 :: GPIO01
#define GPIO_TIMER_INTERRUPT_FROM_FPGA 216      // HEADER_PIN_07 :: GPIO09
#define GPIO_WATCHDOG_INTERRUPT_FROM_FPGA 200   // HEADER_PIN_31 :: GPIO11
#define GPIO_WATCHDOG_INTERRUPT_FROM_CPU 38     // HEADER_PIN_33 :: GPIO13

// gpio-38  (GPIO13              |WDG_INT_FROM_CPU    ) out lo
// gpio-149 (GPIO01              |SPI_INT_FROM_FPGA   ) in  hi IRQ
// gpio-194 (GPIO12              |SPI_INT_FROM_CPU    ) out lo
// gpio-200 (GPIO11              |WDG_INT_FROM_FPGA   ) in  hi IRQ
// gpio-216 (GPIO09              |TIMER_INT_FROM_FPGA ) in  hi IRQ

void isrGpioInit(void);
void isrGpioDestroy(void);

#endif // GPIO_ISR_H

//////////////////////////////////////////////////////////////////////////////////
//                                      //                                      //
// HEADER_PIN_01 :: VDD_3V3_SYS         // HEADER_PIN_02 :: GND                 //
// HEADER_PIN_03 :: I2C1_SDA            // HEADER_PIN_04 :: GND                 //
// HEADER_PIN_05 :: I2C1_SCL            // HEADER_PIN_06 :: GND                 //
// HEADER_PIN_07 :: GPIO09_LS           // HEADER_PIN_08 :: UART1_TXD_LS        //
// HEADER_PIN_09 :: GND                 // HEADER_PIN_10 :: UART1_RXD_LD        //
// HEADER_PIN_11 :: UART1_RTR_LS        // HEADER_PIN_12 :: I2S0_SCLK_LS        //
// HEADER_PIN_13 :: SPI1_SCK_LS         // HEADER_PIN_14 :: GND                 //
// HEADER_PIN_15 :: GPIO12_LS           // HEADER_PIN_16 :: SPI1_CS1_LS         //
// HEADER_PIN_17 :: VDD_3V3_SYS         // HEADER_PIN_18 :: SPI1_CS0_LS         //
// HEADER_PIN_19 :: SPI0_MOSI_LS        // HEADER_PIN_20 :: GND                 //
// HEADER_PIN_21 :: SPI0_MISO_LS        // HEADER_PIN_22 :: SPI1_MISO_LS        //
// HEADER_PIN_23 :: SPI0_CSK_LS         // HEADER_PIN_24 :: SPI0_CS0_LS         //
// HEADER_PIN_25 :: GND                 // HEADER_PIN_26 :: SPI0_CS1_LS         //
// HEADER_PIN_27 :: ID_I2C_SDA          // HEADER_PIN_28 :: ID_I2C_SCL          //
// HEADER_PIN_29 :: GPIO01_LS           // HEADER_PIN_30 :: GND                 //
// HEADER_PIN_31 :: GPIO11_LS           // HEADER_PIN_32 :: GPIO07_LS           //
// HEADER_PIN_33 :: GPIO13_LS           // HEADER_PIN_34 :: GND                 //
// HEADER_PIN_35 :: I2S0_LRCK_LS        // HEADER_PIN_36 :: UART1_CTS_LS        //
// HEADER_PIN_37 :: SPI1_MOSI_LS        // HEADER_PIN_38 :: I2S0_SDIO_LS        //
// HEADER_PIN_39 :: GND                 // HEADER_PIN_40 :: I2S0_SDOUT_LS       //
//                                      //                                      //
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
//
// In/Ount from the code:
//
//////////////////////////////////////////////////////////////////////////////////
//     ___                                  ___
// 01 |3V3|                             02 |5V0|
// 03 |   |                             04 |5V0|
// 05 |   |                             06 |GND|__________________________________
// 07 | t | GPIO09(in)                  08 |   |                        Λ
// 09 |GND|                             10 |   |                        |
// 11 |   |                             12 |   |                        |
// 13 | 1 | SPI1_SCK                    14 |GND|                        |
// 15 | s | GPIO12(out)                 16 |   |                        |
// 17 |3V3|                             18 | 1 | SPI1_CS0               |
// 19 | 0 | SPI0_MOSI                   20 |GND|                        |
// 21 | 0 | SPI0_MISO                   22 | 1 | SPI1_MISO              |
// 23 | 0 | SPI0_CSK                    24 | 0 | SPI0_CS0               |
// 25 |GND|                             26 |   |                        |
// 27 |   |                             28 |   |                        |
// 29 | s | GPIO01(in)                  30 |GND|                        |
// 31 | w | GPIO11(in)                  32 | w | GPIO13(out)            |
// 33 |   |                             34 |GND|                        |
// 35 |   |                             36 |   |                        |
// 37 | 1 | SPI1_MOSI_LS                38 |   |                        |
// 39 |GND|                             40 |___|________________________V_________
//
//////////////////////////////////////////////////////////////////////////////////
