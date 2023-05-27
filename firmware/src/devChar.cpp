//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include <iostream> 		// IO devices :: keyboard
#include <fcntl.h> 			// Open device
#include <string.h> 		// strlem
#include <unistd.h> 		// read/write to the file
#include <cstring> 			// strcmp
#include "devChar.h"

DevChar::DevChar() : m_file_descriptor(0), m_killThread(false)
{
	Console::Info("DevChar :: Construct");
	iceThread = std::thread(&DevChar::iceCOMThread, this);
}
DevChar::~DevChar() 
{
	Console::Info("DevChar :: Destroy");
    if (iceThread.joinable()) 
    {
    	iceThread.join();
   	}
   	else
   	{
		Console::Warning("DevChar :: Thread is not Joinable");
		Console::Warning("DevChar :: Cannot exute to the end");
   	}
}

void 
DevChar::iceCOMThread()
{
	Console::Info("DevChar :: iceCOMThread Launched");

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

	Console::Info("DevChar :: iceCOMThread Termiation");
}

int 
DevChar::device_open(const char* device)
{
	Console::Info("DevChar :: Open iceCOM Device");
	m_file_descriptor = open(device, O_RDWR);
	if (m_file_descriptor < 0)
	{
		Console::Error("DevChar :: Failed to open iceCOM Device");
		return -1;
	}

	return 1;
}

int 
DevChar::device_read()
{
	int ret;
	char console_RX[BUFFER_LENGTH];

	ret = read(m_file_descriptor, console_RX, BUFFER_LENGTH);
	if (ret == -1)
	{
	    Console::Error("DevChar :: Cannot read from kernel space");
	}

	Console::Read(console_RX);

	// clear the buffer
	memset (console_RX, 0, BUFFER_LENGTH);

	return 1;
}

int 
DevChar::device_write()
{
	int ret;
	char console_TX[BUFFER_LENGTH];

	for (int i = 0; i < BUFFER_LENGTH; ++i)
	{
		console_TX[i] = 0;
	}

	Console::Write();
	std::cin.getline(console_TX, BUFFER_LENGTH);

	if(std::strcmp(console_TX, "exit" || "quit") == 0) m_killThread = true;

	ret = write(m_file_descriptor, console_TX, strlen(console_TX)); // Send the string to the LKM
	if (ret == -1)
	{
	    Console::Error("DevChar :: Cannot write to kernel space");
	}

	return 1;
}

int 
DevChar::device_close()
{
	close(m_file_descriptor);
	return 1;
}

bool 
DevChar::terminate()
{
	return m_killThread;
}
