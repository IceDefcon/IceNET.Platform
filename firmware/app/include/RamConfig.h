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
#include "Console.h"

typedef enum
{
    CONFIG_ENGINE,
    CONFIG_BMI160,
    CONFIG_ADXL345,
    CONFIG_AMOUNT
}DeviceType;

struct DeviceConfig
{
    uint8_t size;       // Total bytes sent to FPGA in one SPI/DMA Transfer
    uint8_t ctrl;       // Interface (I2C, SPI, PWM), Read or Write
    uint8_t id;         // Device ID (In case of I2C)
    uint8_t ops;        // Number of Read or Write operations
    uint8_t payload[];  // Combined register addresses and write data
};

class RamConfig : public Console
{
	private:

        int m_fileDescriptor;

        uint8_t m_engineConfig[4];
        DeviceConfig* m_BMI160config;
        DeviceConfig* m_ADXL345config;

        static constexpr const char* DEVICE_PATH = "/dev/IceNETDisk0";
        static constexpr size_t MAX_DMA_TRANSFER_SIZE = 100;
        static constexpr size_t SECTOR_SIZE = 512;

	public:

		RamConfig();
		~RamConfig();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

		char calculate_checksum(char *data, size_t size);
		DeviceConfig* createOperation(char devId, char ctrl, char ops);
		int AssembleData();
        int launchEngine();
        void clearDma();
};

/**
 * ADXL345 :: Regoister config

 * 0x2D ---> 0x00 :: It is recommended to configure the device in standby mode
 * 0x2E ---> 0x00 :: It is recommended that interrupt bits be configured with the interrupts disabled
 * 0x31 ---> 0x08 :: Enable Full Resolution (16bit)
 * 0x2D ---> 0x08 :: Finish config + Enter Measure mode
 */
