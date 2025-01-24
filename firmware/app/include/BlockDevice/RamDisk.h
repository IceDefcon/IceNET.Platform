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

typedef enum
{
    CONFIG_ENGINE,
    CONFIG_BMI160,
    CONFIG_ADXL345,
    CONFIG_AMOUNT
}DeviceType;

struct DeviceConfig
{
    uint8_t size;       // Total bytes of single Device config connected through FPGA bus
    uint8_t ctrl;       // Interface (I2C, SPI, PWM), Read or Write
    uint8_t id;         // Device ID (In case of I2C)
    uint8_t ops;        // Number of Read or Write operations
    uint8_t payload[];  // Combined register addresses and write data
};

class RamDisk
{
	private:

        int m_fileDescriptor;

        RamDisk* m_instance;

        uint8_t* m_engineConfig;
        DeviceConfig* m_BMI160config;
        DeviceConfig* m_ADXL345config;

        static constexpr const char* DEVICE_PATH = "/dev/IceNETDisk0";
        static constexpr size_t MAX_DMA_TRANSFER_SIZE = 100;
        static constexpr size_t SECTOR_SIZE = 512;

	public:

		RamDisk();
		~RamDisk();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        char calculateChecksum(const char* data, size_t size);
		DeviceConfig* createOperation(char devId, char ctrl, char ops);
		int AssembleData();
        void clearDma();

        RamDisk* getInstance();
};

