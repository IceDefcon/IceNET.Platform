#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_


//
// Definitions
//

//Direction type
typedef enum _pin_mux_direction
{
  kPIN_MUX_DirectionInput = 0U,         /* Input direction */
  kPIN_MUX_DirectionOutput = 1U,        /* Output direction */
  kPIN_MUX_DirectionInputOrOutput = 2U  /* Input or output direction */
} pin_mux_direction_t;

//
// API
//
#if defined(__cplusplus)
extern "C" 
{
#endif

  //
  // UART 0 transmit data source select: UART0_TX pin
  //
  #define SOPT5_UART0TXSRC_UART_TX 0x00u

  //
  //Configures pin routing and optionally pin electrical features.
  //
  void BOARD_InitPins(void);

#if defined(__cplusplus)
}
#endif

#endif /* _PIN_MUX_H_ */
