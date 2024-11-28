/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#pragma once

const size_t CHAR_DEVICE_SIZE = 8;
const size_t NETWORK_TRAFFIC_SIZE = 8;
const size_t TCP_SERVER_SIZE = 8;

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
    NetworkTraffic_KernelOutput,
    NetworkTraffic_KILL
} NetworkTraffic_stateType;

typedef enum 
{
    KernelInput_IDLE = 0,
    KernelInput_TX,
    KernelInput_KILL
} KernelInput_stateType;
