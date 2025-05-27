/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#include "debug.h"

static debugCtrlType DebugCtrl[DEBUG_AMOUNT] =
{
    [DEBUG_SPI]  = { .debugFlag = false },
    [DEBUG_CHAR] = { .debugFlag = false },
    [DEBUG_SM]   = { .debugFlag = false },
};

/* CHECK */ bool isDebugEnabled(debugType type)
{
	bool ret = false;

	ret = DebugCtrl[type].debugFlag;

    return ret;
}

/* CTRL */ void ctrlDebug(debugType type, bool flag)
{
    DebugCtrl[type].debugFlag = flag;
}
