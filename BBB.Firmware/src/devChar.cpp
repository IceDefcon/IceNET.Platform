//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include <iostream> 		// IO devices :: keyboard
#include <fcntl.h> 			// Open device   
#include <string.h> 		// for strlem
#include <unistd.h> 		// read/write to the file

#include "devChar.h"

DevChar::DevChar() : m_file_descriptor(0) {}
DevChar::~DevChar() {}

int DevChar::device_open(const char* device)
{
	m_file_descriptor = open(device, O_RDWR);
	if (m_file_descriptor < 0) return -1;

	return 0;
}

int DevChar::device_write()
{
	int ret;
	char console_TX[BUFFER_LENGTH];

	for (int i = 0; i < BUFFER_LENGTH; ++i)
	{
		console_TX[i] = 0;
	}

	Console::Info("TX >> KERNEL");
	Console::Write();
	std::cin.getline(console_TX, BUFFER_LENGTH);

	ret = write(m_file_descriptor, console_TX, strlen(console_TX)); // Send the string to the LKM

	return ret;
}

int DevChar::device_read()
{
	int ret;
	char console_RX[BUFFER_LENGTH];

	ret = read(m_file_descriptor, console_RX, BUFFER_LENGTH);

	Console::Info("RX << KERNEL");
	Console::Read(console_RX);

	// clear the buffer
	memset (console_RX, 0, BUFFER_LENGTH);

	return ret;
}


int DevChar::device_close()
{


	return 0;
}

