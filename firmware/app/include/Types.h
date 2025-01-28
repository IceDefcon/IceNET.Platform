/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#pragma once

#include <string>
#include <array>

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
    IO_LOAD,
    IO_CLEAR,
    IO_AMOUNT
} ioStateType;

typedef enum
{
    CTRL_INIT = 0,
    CTRL_CONFIG,
    CTRL_IDLE,
    CTRL_COMMANDER,
    CTRL_SERVER,
    CTRL_AMOUNT,
} ctrlType;

inline std::string getIoStateString(ioStateType state)
{
    static const std::array<std::string, IO_AMOUNT> ioStateStrings =
    {
        "IO_IDLE",
        "IO_READ",
        "IO_WRITE",
        "IO_LOAD",
        "IO_CLEAR"
    };

    if (state >= 0 && state < IO_AMOUNT)
    {
        return ioStateStrings[state];
    }
    else
    {
        return "UNKNOWN_STATE";
    }
}
