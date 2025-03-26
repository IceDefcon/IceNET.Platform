/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stddef.h> /* size_t */

//////////////////////
//                  //
//                  //
//                  //
//   [CTRL] Types   //
//                  //
//                  //
//                  //
//////////////////////

#define TRANSFER_BUFFER_SIZE 32

#define MANUAL_DMA_TRANSFER_SIZE 4
#define FEEDBACK_DMA_TRANSFER_SIZE 1
#define SENSOR_DMA_TRANSFER_SIZE 12

typedef struct
{
    uint8_t* RxData;
    uint8_t* TxData;
} DmaTransferType;

typedef enum
{
    CTRL_LOCK,
    CTRL_UNLOCK,
    CTRL_AMOUNT
} CtrlType;

#endif // TYPES_H


/**
 *
 * VIN :: VIN
 * A20 :: B20
 * _____________________________________________________________
 *                      Λ                       Λ
 * A19 :: B19           |                       | H7  :: H8
 * A18 :: B18           |                       | H9  :: H10
 * A17 :: B17           |                       | H11 :: H12
 * A16 :: B16           |                       | H13 :: H14
 * A15 :: B15           |                       | H15 :: H16
 * A14 :: B14           |                       | H17 :: H18
 * A13 :: B13           |                       | H19 :: H20
 * A10 :: B10           |                       | H21 :: H22
 * A9  :: B9            | IDC Connector 2x17    | H23 :: H24
 * A8  :: B8            |                       | H25 :: H26
 * A7  :: B7            |                       | H27 :: H28
 * A6  :: B6            |                       | H29 :: H30
 * A5  :: B5            |                       | H31 :: H32
 * C3  :: C4            |                       | H33 :: H34
 * A4  :: B4            |                       | H35 :: H36
 * A3  :: B3            |                       | H37 :: H38
 * B2  :: B1            |                       | H39 :: H40
 * _____________________V_______________________V_______________
 * C2  :: C1
 * D2  :: E1
 * F2  :: F1
 * H2  :: H1
 * J2  :: J1
 * M2  :: M1
 * N2  :: N1
 * P2  :: P1
 * R2  :: R1
 * GND :: GND
 * 3V3 :: 3V3
 * GND :: GND
 *
 */


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
// 23 | 0 | SPI0_CSK                    24 | 0 | SPI0_CS0               | IDC Connector 2x17
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
