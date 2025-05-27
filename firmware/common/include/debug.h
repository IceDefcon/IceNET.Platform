/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>

typedef enum
{
    DEBUG_SPI,
    DEBUG_CHAR,
    DEBUG_SM,
    DEBUG_AMOUNT,
}debugType;

typedef struct
{
    bool debugFlag;
}debugCtrlType;

/* CHECK */ bool isDebugEnabled(debugType type);
/* CTRL */ void ctrlDebug(debugType type, bool flag);

#endif // DEBUG_H
