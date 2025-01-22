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
    NetworkTraffic_Input,
    NetworkTraffic_Output,
    NetworkTraffic_KILL
} NetworkTraffic_stateType;

typedef enum 
{
    Input_IDLE = 0,
    Input_TX,
    Input_KILL
} Input_stateType;
