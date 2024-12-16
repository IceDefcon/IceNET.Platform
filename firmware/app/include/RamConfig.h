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

#define DEVICE_PATH "/dev/IceNETDisk0" // Adjust based on your ramdisk naming
#define SECTOR_SIZE 512  // Sector size in bytes

class RamConfig : public Console
{
	private:

		int m_test_1;
		int m_test_2;

    struct OperationType 
    {
        char header;    // Unique ID of the operation
        char size;       // Total bytes sent to FPGA in one SPI/DMA Transfer (change to int)
        char ctrl;      // Interface (I2C, SPI, PWM), Read or Write
        char devId;     // Device ID (e.g., for I2C)
        char ops;       // Number of Read or Write operations
        char checksum;  // 8-bit checksum
        char payload[]; // Combined register addresses and write data
    };

	public:

		RamConfig();
		~RamConfig();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

		char calculate_checksum(char *data, size_t size);
		OperationType* createOperation(char devId, char ctrl, char ops);
		int Execute();
};

/**
 * ADXL345 :: Regoister config

 * 0x2D ---> 0x00 :: It is recommended to configure the device in standby mode
 * 0x2E ---> 0x00 :: It is recommended that interrupt bits be configured with the interrupts disabled
 * 0x31 ---> 0x08 :: Enable Full Resolution (16bit)
 * 0x2D ---> 0x08 :: Finish config + Enter Measure mode
 */
