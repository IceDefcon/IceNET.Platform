//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include <iostream> 		// IO devices :: keyboard
#include <string>
#include "device.h"

// #include <stdio.h>
// #include <stdlib.h>
// #include <errno.h>
#include <fcntl.h> 			// Open device   
#include <string.h> 		// for strlem
#include <unistd.h> 		// read/write to the file

using namespace std;

int Device::device_open()
{
	m_file_device = open("/dev/chardev", O_RDWR);             // Open the device with read/write access
	if (m_file_device < 0) return -1;

	return 0;
}

int Device::device_read()
{
	int ret;
	char console_RX[256];

	ret = read(m_file_device, console_RX, BUFFER_LENGTH);

	cout << "Rx essage  is: " << console_RX  << endl;
	cout << "Rx pointer is: " << &console_RX << endl;

	return 0;
}

int Device::device_write()
{
	int ret;
	char console_TX[256];

	cout << "IceNET ---> ";
	cin  >> console_TX;

	ret = write(m_file_device, console_TX, strlen(console_TX)); // Send the string to the LKM

	return 0;
}

int Device::device_exit()
{


	return 0;
}

