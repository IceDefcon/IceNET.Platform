/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#pragma once

enum Status
{
	OK,
	ERROR,
	UNKNOWN
};

typedef enum 
{
    IDLE = 0,
    inputCOM_TRANSFER,
    iceNET_TRANSFER
} stateType;
