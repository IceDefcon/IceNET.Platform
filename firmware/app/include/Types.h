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
    NetworkTraffic_IDLE = 0,
    NetworkTraffic_Kernel_IN,
    NetworkTraffic_Kernel_OUT
} NetworkTraffic_stateType;

typedef enum 
{
    Kernel_IN_IDLE = 0,
    Kernel_IN_TX
} Kernel_IN_stateType;