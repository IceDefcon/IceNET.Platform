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
    Kernel_IN_TRANSFER,
    Kernel_OUT_TRANSFER
} stateType;
