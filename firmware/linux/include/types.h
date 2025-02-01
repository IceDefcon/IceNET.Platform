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

typedef struct
{
    uint8_t* RxData;
    uint8_t* TxData;
    size_t size;
} DmaTransferType;

#endif // TYPES_H
