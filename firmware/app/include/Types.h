/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#pragma once

const size_t CHAR_DEVICE_SIZE = 32;
const size_t CHAR_CONSOLE_SIZE = 32;
const size_t NETWORK_TRAFFIC_SIZE = 32;
const size_t TCP_SERVER_SIZE = 32;

enum Status
{
	OK,
	ERROR,
	UNKNOWN
};

typedef enum 
{
    NetworkTraffic_IDLE = 0,
    NetworkTraffic_KernelInput,
    NetworkTraffic_KernelOutput
} NetworkTraffic_stateType;

typedef enum 
{
    KernelInput_IDLE = 0,
    KernelInput_TX
} KernelInput_stateType;