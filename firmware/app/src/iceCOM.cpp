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
	Debug::Info("iceCOM :: Initialise iceCOM Module");
}

iceCOM::~iceCOM() 
{
	Debug::Info("iceCOM :: Destroying iceCOM Module");
    if (m_iceThread.joinable()) 
    {
    	m_iceThread.join();
   	}
}

void iceCOM::initThread()
{
	Debug::Info("iceCOM :: Init the iceCOMThread");
	m_iceThread = std::thread(&iceCOM::iceCOMThread, this);
}

void iceCOM::iceCOMThread()
{
	Debug::Info("iceCOM :: Enter iceCOMThread");

    while(!m_killThread) 
    {
        //////////////////
        // 				//
        // 				//
        // 				//
        // Thread  Work //
        // 				//
        // 				//
        // 				//
        //////////////////

    	if(OK != device_write())
    	{
			Debug::Error("iceCOM :: Cannot write into the console");
    	}

    	if(OK != device_read())
    	{
			Debug::Error("iceCOM :: Cannot read from the console");
    	}
    }

	Debug::Info("iceCOM :: Terminate iceCOMThread");
}

int iceCOM::device_open(const char* device)
{
	m_file_descriptor = open(device, O_RDWR);

	if (m_file_descriptor < 0)
	{
		Debug::Error("iceCOM :: Failed to open Device");
		m_killThread = true;
		return ERROR;
	}
	else
	{
		Debug::Info("iceCOM :: Device opened successfuly");
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
	    Debug::Error("iceCOM :: Cannot read from kernel space");
	    return ERROR;
	}

	Debug::Read(console_RX);

	// clear the buffer
	memset (console_RX, 0, BUFFER_LENGTH);

	return OK;
}

int iceCOM::device_write()
{
	int i;
	int ret;
	char console_TX[BUFFER_LENGTH];

	for (size_t i = 0; i < BUFFER_LENGTH; ++i)
	{
		console_TX[i] = 0;
	}

	Debug::Write();
	/* Get console characters */
	std::cin.getline(console_TX, BUFFER_LENGTH);

	if (std::strcmp(console_TX, "exit") == 0) 
	{
    	m_killThread = true;
	}
	else if (std::strcmp(console_TX, "mag") == 0) /* Magnetometer */
	{
	    for (i = 0; i < 8; i++) 
	    {
	        console_TX[i] = 0x04 + i;
	    }
	}
	else if (std::strcmp(console_TX, "hal") == 0) /* Hall resistance */
	{
	    for (i = 0; i < 2; i++) 
	    {
	        console_TX[i] = 0x0A + i;
	    }
	}
	else if (std::strcmp(console_TX, "gyr") == 0) /* Magnetometer */
	{
	    for (i = 0; i < 6; i++) 
	    {
	        console_TX[i] = 0x0C + i;
	    }
	}
	else if (std::strcmp(console_TX, "acc") == 0) /* Magnetometer */
	{
	    for (i = 0; i < 6; i++) 
	    {
	        console_TX[i] = 0x12 + i;
	    }
	}
	else
	{
		Debug::Error("iceCOM :: Command not found");
	}

	ret = write(m_file_descriptor, console_TX, strlen(console_TX)); // Send the string to the LKM
	if (ret == -1)
	{
	    Debug::Error("iceCOM :: Cannot write to kernel space");
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
