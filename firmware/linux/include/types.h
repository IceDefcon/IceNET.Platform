/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#ifndef SPI_TYPES_H
#define SPI_TYPES_H

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

#define TRANSFER_BUFFER_SIZE 8

typedef struct
{
    volatile char *RxData;
    volatile char *TxData;
    size_t length;
} DataTransfer;

#endif // SPI_TYPES_H
