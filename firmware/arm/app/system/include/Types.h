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

typedef enum
{
	OK,
	ERROR,
	UNKNOWN
} Status;

typedef enum
{
    CMD_DMA_NORMAL,
    CMD_DMA_SENSOR,
    CMD_DMA_SINGLE,
    CMD_DMA_CUSTOM,
    CMD_RAMDISK_CONFIG,
    CMD_RAMDISK_CLEAR,
    CMD_DEBUG_ENABLE,
    CMD_DEBUG_DISABLE,
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
    VECTOR_OFFLOAD_EXTERNAL,
    VECTOR_TRIGGER,
    VECTOR_F1,
    VECTOR_F2,
    VECTOR_F3,
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
    IO_COM_CALIBRATION,
    IO_AMOUNT
} ioStateType;

typedef enum
{
    DRONE_CTRL_IDLE = 0,
    DRONE_CTRL_INIT,
    DRONE_CTRL_CONFIG,
    DRONE_CTRL_RAMDISK_PERIPHERALS,
    DRONE_CTRL_RAMDISK_ACTIVATE_DMA,
    DRONE_CTRL_DMA_SINGLE,
    DRONE_CTRL_MAIN,
    DRONE_CTRL_AMOUNT,
} droneCtrlStateType;

typedef enum
{
   PWM_EXE,
   PWM_UP,
   PWM_DOWN,
   PWM_AMOUNT
}pwmType;

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

