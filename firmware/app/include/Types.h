/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#pragma once

const size_t CHAR_DEVICE_SIZE = 8;
const size_t TCP_SERVER_SIZE = 8;

enum Status
{
	OK,
	ERROR,
	UNKNOWN
};

typedef enum 
{
    COMMANDER_IDLE = 0,
    COMMANDER_READ,
    COMMANDER_WRITE,
    COMMANDER_DEAD,
    COMMANDER_LOAD,
    COMMANDER_CLEAR,
    COMMANDER_AMOUNT
} commanderStateType;
