//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include <iostream> 		// IO devices :: keyboard
#include <fcntl.h> 			// Open device   
#include <string.h> 		// for strlem
#include <unistd.h> 		// read/write to the file

#include "device.h"

Device::Device() : m_file_device(0) {}

int Device::device_open(const char* device)
{
	m_file_device = open(device, O_RDWR);
	if (m_file_device < 0) return -1;

	return 0;
}

int Device::device_write()
{
	int ret;
	char console_TX[256];

	for (int i = 0; i < 256; ++i)
	{
		console_TX[i] = 0;
	}

	Console::Info("TX >> KERNEL");
	Console::Write();
	cin.getline(console_TX,256);

	ret = write(m_file_device, console_TX, strlen(console_TX)); // Send the string to the LKM

	return 0;
}

int Device::device_read()
{
	int ret;
	char console_RX[256];

	ret = read(m_file_device, console_RX, BUFFER_LENGTH);

	Console::Info("RX << KERNEL");
	Console::Read(console_RX);

	// clear the buffer
	memset (console_RX,0,256);

	return 0;
}


int Device::device_close()
{


	return 0;
}

