//
//
//
#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"

#define PIN0_IDX                         0u   /*!< Pin number for pin 0 in a port */
#define PIN1_IDX                         1u   /*!< Pin number for pin 1 in a port */
#define PIN4_IDX                         4u   /*!< Pin number for pin 4 in a port */
#define PIN5_IDX                         5u   /*!< Pin number for pin 5 in a port */
#define PIN12_IDX                       12u   /*!< Pin number for pin 12 in a port */
#define PIN13_IDX                       13u   /*!< Pin number for pin 13 in a port */
#define PIN14_IDX                       14u   /*!< Pin number for pin 14 in a port */
#define PIN15_IDX                       15u   /*!< Pin number for pin 15 in a port */
#define PIN16_IDX                       16u   /*!< Pin number for pin 16 in a port */
#define PIN17_IDX                       17u   /*!< Pin number for pin 17 in a port */
#define PIN18_IDX                       18u   /*!< Pin number for pin 18 in a port */
#define PIN21_IDX                       21u   /*!< Pin number for pin 21 in a port */
#define PIN22_IDX                       22u   /*!< Pin number for pin 22 in a port */
#define SOPT5_UART0TXSRC_UART_TX      0x00u   /*!< UART 0 transmit data source select: UART0_TX pin */

//
// FUNCTION
//
void BOARD_InitPins(void) 
{
  CLOCK_EnableClock(kCLOCK_PortA);                           /* Port A Clock Gate Control: Clock enabled */
  CLOCK_EnableClock(kCLOCK_PortB);                           /* Port B Clock Gate Control: Clock enabled */
  CLOCK_EnableClock(kCLOCK_PortC);                           /* Port C Clock Gate Control: Clock enabled */
  CLOCK_EnableClock(kCLOCK_PortD);                           /* Port C Clock Gate Control: Clock enabled */

  //
  // GPIO :: LED Control
  //
  // This is controled in MAIN :: Bare Metal
  //

  //
  // SPI :: 0 
  //
  PORT_SetPinMux(PORTD, 0U, kPORT_MuxAlt2);                  /* PORTD0 (pin 93) is configured as SPI0_PCS0 */
  PORT_SetPinMux(PORTD, 1U, kPORT_MuxAlt2);                  /* PORTD1 (pin 94) is configured as SPI0_SCK */
  PORT_SetPinMux(PORTD, 2U, kPORT_MuxAlt2);                  /* PORTD2 (pin 95) is configured as SPI0_SOUT */
  PORT_SetPinMux(PORTD, 3U, kPORT_MuxAlt2);                  /* PORTD3 (pin 96) is configured as SPI0_SIN */

  //
  // SPI :: 1
  //
  PORT_SetPinMux(PORTD, 4U, kPORT_MuxAlt7);                  /* PORTD4 (pin 97) is configured as SPI1_PCS0 */
  PORT_SetPinMux(PORTD, 5U, kPORT_MuxAlt7);                  /* PORTD5 (pin 98) is configured as SPI1_SCK */
  PORT_SetPinMux(PORTD, 6U, kPORT_MuxAlt7);                  /* PORTD6 (pin 99) is configured as SPI1_SOUT */
  PORT_SetPinMux(PORTD, 7U, kPORT_MuxAlt7);                  /* PORTD7 (pin 100) is configured as SPI1_SIN */

  //
  // RMIN :: IO
  //
  PORT_SetPinMux(PORTA, PIN12_IDX, kPORT_MuxAlt4);           /* PORTA12 (pin 42) is configured as RMII0_RXD1 */
  PORT_SetPinMux(PORTA, PIN13_IDX, kPORT_MuxAlt4);           /* PORTA13 (pin 43) is configured as RMII0_RXD0 */
  PORT_SetPinMux(PORTA, PIN14_IDX, kPORT_MuxAlt4);           /* PORTA14 (pin 44) is configured as RMII0_CRS_DV */
  PORT_SetPinMux(PORTA, PIN15_IDX, kPORT_MuxAlt4);           /* PORTA15 (pin 45) is configured as RMII0_TXEN */
  PORT_SetPinMux(PORTA, PIN16_IDX, kPORT_MuxAlt4);           /* PORTA16 (pin 46) is configured as RMII0_TXD0 */
  PORT_SetPinMux(PORTA, PIN17_IDX, kPORT_MuxAlt4);           /* PORTA17 (pin 47) is configured as RMII0_TXD1 */

  const port_pin_config_t porta4_pin38_config = {
    kPORT_PullUp,                                            /* Internal pull-up resistor is enabled */
    kPORT_FastSlewRate,                                      /* Fast slew rate is configured */
    kPORT_PassiveFilterDisable,                              /* Passive filter is disabled */
    kPORT_OpenDrainDisable,                                  /* Open drain is disabled */
    kPORT_HighDriveStrength,                                 /* High drive strength is configured */
    kPORT_MuxAsGpio,                                         /* Pin is configured as PTA4 */
    kPORT_UnlockRegister                                     /* Pin Control Register fields [15:0] are not locked */
  };

  PORT_SetPinConfig(PORTA, PIN4_IDX, &porta4_pin38_config);  /* PORTA4 (pin 38) is configured as PTA4 */
  PORT_SetPinMux(PORTA, PIN5_IDX, kPORT_MuxAlt4);            /* PORTA5 (pin 39) is configured as RMII0_RXER */

  const port_pin_config_t portb0_pin53_config = {
    kPORT_PullUp,                                            /* Internal pull-up resistor is enabled */
    kPORT_FastSlewRate,                                      /* Fast slew rate is configured */
    kPORT_PassiveFilterDisable,                              /* Passive filter is disabled */
    kPORT_OpenDrainEnable,                                   /* Open drain is enabled */
    kPORT_LowDriveStrength,                                  /* Low drive strength is configured */
    kPORT_MuxAlt4,                                           /* Pin is configured as RMII0_MDIO */
    kPORT_UnlockRegister                                     /* Pin Control Register fields [15:0] are not locked */
  };

  PORT_SetPinConfig(PORTB, PIN0_IDX, &portb0_pin53_config);  /* PORTB0 (pin 53) is configured as RMII0_MDIO */
  PORT_SetPinMux(PORTB, PIN1_IDX, kPORT_MuxAlt4);            /* PORTB1 (pin 54) is configured as RMII0_MDC */

  //
  // UART 0
  //
  PORT_SetPinMux(PORTB, PIN16_IDX, kPORT_MuxAlt3);           /* PORTB16 (pin 62) is configured as UART0_RX */
  PORT_SetPinMux(PORTB, PIN17_IDX, kPORT_MuxAlt3);           /* PORTB17 (pin 63) is configured as UART0_TX */
  
  PORT_SetPinMux(PORTC, PIN16_IDX, kPORT_MuxAlt4);           /* PORTC16 (pin 90) is configured as ENET0_1588_TMR0 */
  PORT_SetPinMux(PORTC, PIN17_IDX, kPORT_MuxAlt4);           /* PORTC17 (pin 91) is configured as ENET0_1588_TMR1 */
  PORT_SetPinMux(PORTC, PIN18_IDX, kPORT_MuxAlt4);           /* PORTC18 (pin 92) is configured as ENET0_1588_TMR2 */

  SIM->SOPT5 = ((SIM->SOPT5 &
    (~(SIM_SOPT5_UART0TXSRC_MASK)))                          /* Mask bits to zero which are setting */
      | SIM_SOPT5_UART0TXSRC(SOPT5_UART0TXSRC_UART_TX));     /* UART 0 transmit data source select: UART0_TX pin */
}
