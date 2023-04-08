#ifndef __GPIO_H__
#define __GPIO_H__

#include "common.h"

///////////////////////////////////////////////////////////////////////////////////
// GPIO ---> For LED status control :: Manual
///////////////////////////////////////////////////////////////////////////////////
//                                          // K64 Sub-Family Reference Manual
//                                          // ==================================
//                                          //                      (page number)
#define SIM_SCGC5 (*(int *)0x40048038u)     // Clock gate 5                 (314)
#define SIM_SCGC5_PORTB 10                  // Open gate PORTB              (314)

#define PORTB_PCR21 (*(int *)0x4004A054u)   // Pin Control Register         (277)
#define PORTB_PCR21_MUX 8                   // Mux "001"                    (282)
#define PORTB_PCR22 (*(int *)0x4004A058u)   // Pin Control Register         (277)
#define PORTB_PCR22_MUX 8                   // Mux "001"                    (282)

#define GPIOB_PDDR (*(int *)0x400FF054u)    // Port Data Direction Register (1760)
#define GPIOB_PDOR (*(int *)0x400FF040u)    // Port Data Output Register    (1759)
#define PIN_21_N 21                         // PTB21 --> Blue LED           (1761)
#define PIN_22_N 22                         // PTB22 --> Red  LED           (1761)

#define SIM_SCGC5 (*(int *)0x40048038u)     // Clock gate 5                 (314)
#define SIM_SCGC5_PORTD 12                  // Open gate PORTD              (314)

#define PORTD_PCR1 (*(int *)0x4004C004u)    // Pin Control Register         (277)
#define PORTD_PCR1_MUX 8                    // Mux "001"                    (282)

#define GPIOD_PDDR (*(int *)0x400FF0D4u)    // Port Data Direction Register (1760)
#define GPIOD_PDOR (*(int *)0x400FF0C0u)    // Port Data Output Register    (1759)
#define PIN_SPI_N 1                         // PTB21 --> Blue LED           (1761)


void gpio_init();
void Blinky_task();


#endif /* __GPIO__ */