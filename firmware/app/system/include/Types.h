/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#pragma once

#include <string>
#include <array>
#include <iostream>
#include <vector>
#include <iomanip>  // For std::hex and std::setw
#include <memory>

const size_t CMD_LENGTH = 2;
const size_t IO_TRANSFER_SIZE = 8;

enum Status
{
	OK,
	ERROR,
	UNKNOWN
};

typedef enum
{
    CMD_FPGA_CONFIG,
    CMD_DMA_RECONFIG,
    CMD_DMA_CLEAR,
    CMD_AMOUNT
} commandType;

typedef enum 
{
    IO_IDLE = 0,
    IO_COM_WRITE,
    IO_COM_READ,
    IO_AMOUNT
} ioStateType;

typedef enum
{
    CTRL_INIT = 0,
    CTRL_DMA_LONG,
    CTRL_DMA_SINGLE,
    CTRL_MAIN,
    CTRL_AMOUNT,
} ctrlType;

typedef enum
{
   PWM_EXE,
   PWM_UP,
   PWM_DOWN,
   PWM_AMOUNT
}pwmType;

inline std::string getIoStateString(ioStateType state)
{
    static const std::array<std::string, IO_AMOUNT> ioStateStrings =
    {
        "IO_IDLE",
        "IO_COM_WRITE",
        "IO_COM_READ",
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

inline void printHexBuffer(std::vector<char>* buffer)
{
    std::cout << "[INFO] [HEX] Data in the buffer: ";
    for (size_t i = 0; i < buffer->size(); ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)(*buffer)[i] << " ";
    }
    std::cout << std::endl;
}

inline void printSharedBuffer(std::shared_ptr<std::vector<uint8_t>> buffer)
{
    std::cout << "[INFO] [SHARED] Data in the buffer: ";
    for (size_t i = 0; i < buffer->size(); ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)(*buffer)[i] << " ";
    }
    std::cout << std::endl;
}

