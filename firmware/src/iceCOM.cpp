//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include <iostream> 		// IO devices :: keyboard
#include <fcntl.h> 			// Open device
#include <string.h> 		// strlem
#include <unistd.h> 		// read/write to the file
#include <cstring> 			// strcmp
#include "iceCOM.h"

iceCOM::iceCOM() : 
m_file_descriptor(0), 
m_killThread(false),
m_BUFFER_LENGTH(256)
{
	Debug::Info("iceCOM :: Construct");
	m_iceThread = std::thread(&iceCOM::iceCOMThread, this);
}

iceCOM::~iceCOM() 
{
	Debug::Info("iceCOM :: Destroy");
    if (m_iceThread.joinable()) 
    {
    	m_iceThread.join();
   	}
   	else
   	{
		Debug::Warning("iceCOM :: Thread is not Joinable");
   	}
}

void 
iceCOM::iceCOMThread()
{
	Debug::Info("iceCOM :: iceCOMThread Launched");

    while (!m_killThread) 
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

    	device_write();
        device_read();
    }

	Debug::Info("iceCOM :: iceCOMThread Termiation");
}

int 
iceCOM::device_open(const char* device)
{
	Debug::Info("iceCOM :: Open iceCOM Device");
	m_file_descriptor = open(device, O_RDWR);
	if (m_file_descriptor < 0)
	{
		Debug::Error("iceCOM :: Failed to open iceCOM Device");
		return -1;
	}

	return 1;
}

int 
iceCOM::device_read()
{
	int ret;
	char console_RX[m_BUFFER_LENGTH];

	ret = read(m_file_descriptor, console_RX, m_BUFFER_LENGTH);
	if (ret == -1)
	{
	    Debug::Error("iceCOM :: Cannot read from kernel space");
	}

	Debug::Read(console_RX);

	// clear the buffer
	memset (console_RX, 0, m_BUFFER_LENGTH);

	return 1;
}

int 
iceCOM::device_write()
{
	int ret;
	char console_TX[m_BUFFER_LENGTH];

	for (int i = 0; i < m_BUFFER_LENGTH; ++i)
	{
		console_TX[i] = 0;
	}

	Debug::Write();
	std::cin.getline(console_TX, m_BUFFER_LENGTH);

	if (std::strcmp(console_TX, "exit") == 0) 
	{
    	m_killThread = true;
	}

	ret = write(m_file_descriptor, console_TX, strlen(console_TX)); // Send the string to the LKM
	if (ret == -1)
	{
	    Debug::Error("iceCOM :: Cannot write to kernel space");
	}

	return 1;
}

int 
iceCOM::device_close()
{
	close(m_file_descriptor);
	return 1;
}

bool 
iceCOM::terminate()
{
	return m_killThread;
}
