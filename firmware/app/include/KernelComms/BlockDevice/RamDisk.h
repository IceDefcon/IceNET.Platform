/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include "Types.h"

#define DEVICE_PATH "/dev/IceNETDisk0"
#define SECTOR_SIZE 512
#define HEADER_SIZE 0x04
#define SCRAMBLE_BYTE 0x77

typedef enum
{
    CONFIG_ENGINE,
    CONFIG_BMI160,
    CONFIG_ADXL345,
    CONFIG_AMOUNT
}DeviceType;

typedef struct
{
    uint8_t size;       // Total bytes of single Device config connected through FPGA bus
    uint8_t ctrl;       // Interface (I2C, SPI, PWM), Read or Write
    uint8_t id;         // Device ID (In case of I2C)
    uint8_t ops;        // Number of Read or Write operations
    uint8_t payload[];  // Combined register addresses and write data
}DeviceConfigType;

typedef struct
{
    uint8_t id;  // Device ID (e.g., I2C address)
    uint8_t ctrl; // OFFLOAD_CTRL :: 8-bits
    std::vector<std::pair<uint8_t, uint8_t>> registers;
}SensorConfigType;

class RamDisk
{
	private:

        int m_fileDescriptor;

        RamDisk* m_instance;

        std::vector<uint8_t> m_engineConfig;
        std::vector<SensorConfigType> m_devices;
        std::vector<DeviceConfigType*> m_deviceConfigs; // Stores multiple device configurations

	public:

		RamDisk();
		~RamDisk();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        uint8_t calculateChecksum(const uint8_t* data, size_t size);
		DeviceConfigType* createOperation(uint8_t id, uint8_t ctrl, uint8_t ops);
		int assembleConfig();
        int sendConfig();
        void clearDma();
};
