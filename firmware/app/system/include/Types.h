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
    CMD_DMA_NORMAL,
    CMD_DMA_SENSOR,
    CMD_DMA_SINGLE,
    CMD_DMA_CUSTOM,
    CMD_RAMDISK_CONFIG,
    CMD_RAMDISK_CLEAR,
    CMD_FPGA_RESET,
    CMD_AMOUNT
} commandType;

typedef enum
{
    VECTOR_RESERVED,
    VECTOR_OFFLOAD_PRIMARY,
    VECTOR_ENABLE,
    VECTOR_DISABLE,
    VECTOR_START,
    VECTOR_STOP,
    VECTOR_READ,
    VECTOR_OFFLOAD_SECONDARY,
    VECTOR_F1,
    VECTOR_F2,
    VECTOR_RETURN,
    VECTOR_UNUSED_11,
    VECTOR_UNUSED_12,
    VECTOR_UNUSED_13,
    VECTOR_UNUSED_14,
    VECTOR_UNUSED_15,
    VECTOR_AMOUNT
}interruptVectorType;

typedef enum 
{
    IO_COM_IDLE = 0,
    IO_COM_WRITE,
    IO_COM_WRITE_ONLY,
    IO_COM_READ,
    IO_COM_READ_ONLY,
    IO_AMOUNT
} ioStateType;

typedef enum
{
    CTRL_INIT = 0,
    CTRL_RAMDISK_PERIPHERALS,
    CTRL_RAMDISK_ACTIVATE_DMA,
    CTRL_DMA_SINGLE,
    CTRL_MEAS_TEST,
    CTRL_MAIN,
    CTRL_AMOUNT,
} droneCtrlStateType;

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
        "IO_COM_IDLE",
        "IO_COM_WRITE",
        "IO_COM_WRITE_ONLY",
        "IO_COM_READ",
        "IO_COM_READ_ONLY",
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

