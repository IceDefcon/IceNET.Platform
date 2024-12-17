/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "Types.h"
#include "Console.h"

typedef enum
{
    CONFIG_BMI160,
    CONFIG_ADXL345,
    CONFIG_AMOUNT
}DeviceType;


typedef struct
{
    char size;      // Total bytes sent to FPGA in one SPI/DMA Transfer (change to int)
    char ctrl;      // Interface (I2C, SPI, PWM), Read or Write
    char id;        // Device ID (In case of I2C)
    char ops;       // Number of Read or Write operations
    char payload[]; // Combined register addresses and write data
}DeviceConfig;

class RamConfig : public Console
{
	private:

        int m_file_descriptor;
        char m_goDma;

        char m_first_sector[1];
        char m_forth_sector[4];

        DeviceConfig* m_pDevice[CONFIG_AMOUNT];

	public:

        static constexpr const char* DEVICE_PATH = "/dev/IceNETDisk0";
        static constexpr int MAX_DMA_TRANSFER_SIZE = 100;
        static constexpr int SECTOR_SIZE = 512;

		RamConfig();
		~RamConfig();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

		char calculate_checksum(char *data, size_t size);
		DeviceConfig* createOperation(char devId, char ctrl, char ops);
		int AssembleData();
};

/**
 * ADXL345 :: Regoister config

 * 0x2D ---> 0x00 :: It is recommended to configure the device in standby mode
 * 0x2E ---> 0x00 :: It is recommended that interrupt bits be configured with the interrupts disabled
 * 0x31 ---> 0x08 :: Enable Full Resolution (16bit)
 * 0x2D ---> 0x08 :: Finish config + Enter Measure mode
 */
