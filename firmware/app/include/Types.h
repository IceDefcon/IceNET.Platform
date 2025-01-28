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
    IO_IDLE = 0,
    IO_READ,
    IO_WRITE,
    IO_DEAD,
    IO_LOAD,
    IO_CLEAR,
    IO_TEST,
    IO_AMOUNT
} ioStateType;

typedef enum
{
    CTRL_INIT = 0,
    CTRL_CONFIG,
    CTRL_IDLE,
    CTRL_INPUT,
    CTRL_OUTPUT,
} ctrlType;
