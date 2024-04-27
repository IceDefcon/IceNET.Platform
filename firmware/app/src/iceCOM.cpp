/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <iostream> 		// IO devices :: keyboard
#include <fcntl.h> 			// Open device
#include <string.h> 		// strlem
#include <unistd.h> 		// read/write to the file
#include <cstring> 			// strcmp
#include <termios.h> 		// terminal settings
#include "iceCOM.h"

iceCOM::iceCOM(): 
m_file_descriptor(0), 
m_killThread(false)
{
	Debug::Info("[iceCOM] Initialise iceCOM Module");
}

iceCOM::~iceCOM() 
{
	Debug::Info("[iceCOM] Destroying iceCOM Module");
    if (m_iceThread.joinable()) 
    {
    	m_iceThread.join();
   	}
}

void iceCOM::initThread()
{
	Debug::Info("[iceCOM] Init the iceCOMThread");
	m_iceThread = std::thread(&iceCOM::iceCOMThread, this);
}

void iceCOM::iceCOMThread()
{
	Debug::Info("[iceCOM] Enter iceCOMThread");

    while(!m_killThread) 
    {
    	/*!
    	 * 
    	 * ----===[ TODO ]===----
    	 * 
    	 * In addition we can print some feedback
    	 * information on the [ RX ] console 
    	 * when SPI transfers are executed
    	 * 
    	 */

    	if(OK != device_write())
    	{
			Debug::Error("[iceCOM] Cannot write into the console");
    	}
    	else
    	{
    		/**
    		 * 
    		 * At the moment Feedback only print info 
    		 * about successfully transfered command 
    		 * 
    		 */
	    	if(OK != device_read())
	    	{
				Debug::Error("[iceCOM] Cannot read from the console");
	    	}
    	}
    }

	Debug::Info("[iceCOM] Terminate iceCOMThread");
}

int iceCOM::device_open(const char* device)
{
	m_file_descriptor = open(device, O_RDWR);

	if (m_file_descriptor < 0)
	{
		Debug::Error("[iceCOM] Failed to open Device");
		m_killThread = true;
		return ERROR;
	}
	else
	{
		Debug::Info("[iceCOM] Device opened successfuly");
		initThread();
	}

	return OK;
}

int iceCOM::device_read()
{
	int ret;
	char console_RX[BUFFER_LENGTH];

	ret = read(m_file_descriptor, console_RX, BUFFER_LENGTH);
	if (ret == -1)
	{
	    Debug::Error("[iceCOM] Cannot read from kernel space");
	    return ERROR;
	}
	else
	{
		Debug::Read(console_RX);
	}

	memset (console_RX, 0, BUFFER_LENGTH);

	return OK;
}

int iceCOM::device_write()
{
	int ret = -1;
	char console_TX[6];

	Debug::Write();
	/* Get console characters */
	std::cin.getline(console_TX, BUFFER_LENGTH);

	if (std::strcmp(console_TX, "exit") == 0) 
	{
    	m_killThread = true;
    	/**
    	 * 
    	 *  TODO
    	 * 
    	 * Consider where to close char deveice
    	 * 
    	 */
    	// iceCOM::device_close();
	}
	else if (std::strcmp(console_TX, "mag") == 0) /* Magnetometer */
	{
		console_TX[0] = 0x04;
		console_TX[1] = 0x05;
		console_TX[2] = 0x06;
		console_TX[3] = 0x07;
		console_TX[4] = 0x08;
		console_TX[5] = 0x09;
		ret = write(m_file_descriptor, console_TX, sizeof(console_TX)/sizeof(console_TX[0])); // Send the string to the LKM
	}
	else
	{
		Debug::Error("[iceCOM] Command not found");
		ret = -1;
	}

	if (ret == -1)
	{
	    Debug::Error("[iceCOM] Cannot write to kernel space");
	    return ERROR;
	}

	return OK;
}

int iceCOM::device_close()
{
	close(m_file_descriptor);
	return OK;
}

bool iceCOM::terminate()
{
	return m_killThread;
}
