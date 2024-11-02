/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
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

#define TRANSFER_BUFFER_SIZE 8

typedef struct
{
    volatile char *RxData;
    volatile char *TxData;
    size_t length;
} DataTransfer;

#endif // TYPES_H
