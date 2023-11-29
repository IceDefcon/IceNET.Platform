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
#include "iceCOM.h"

iceCOM::iceCOM(): 
m_file_descriptor(0), 
m_killThread(false)
{
	Debug::Info("iceCOM :: Initialising iceCOM Module");
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
	Debug::Info("iceCOM :: Init iceCOMThread");
	m_iceThread = std::thread(&iceCOM::iceCOMThread, this);
}

void iceCOM::iceCOMThread()
{
	Debug::Info("iceCOM :: Start iceCOMThread");

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

	Debug::Info("iceCOM :: iceCOMThread Terminate");
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

	Debug::Info("iceCOM :: Device opened successfuly");
	initThread();

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

// int iceCOM::device_write()
// {
// 	int ret;
// 	char console_TX[BUFFER_LENGTH];

// 	for (size_t i = 0; i < BUFFER_LENGTH; ++i)
// 	{
// 		console_TX[i] = 0;
// 	}

// 	Debug::Write();
// 	std::cin.getline(console_TX, BUFFER_LENGTH);

// 	if (std::strcmp(console_TX, "exit") == 0) 
// 	{
//     	m_killThread = true;
// 	}

//     char inputChar;

//     // Read a single character
//     std::cin >> inputChar;

//     if (inputChar == 'q') 
//     {
//         m_killThread = true;
//     }

// 	ret = write(m_file_descriptor, console_TX, strlen(console_TX)); // Send the string to the LKM
// 	if (ret == -1)
// 	{
// 	    Debug::Error("iceCOM :: Cannot write to kernel space");
// 	    return ERROR;
// 	}

// 	return OK;
// }

int iceCOM::device_write()
{
	Debug::Write();

    char inputChar = '\0';

    // Read a single character
    std::cin >> inputChar;

    if (inputChar == 'q') 
    {
        m_killThread = true;
    }

    // Write the single character to the character device
    int ret = write(m_file_descriptor, &inputChar, 1);

    if (ret == -1)
    {
        // Handle write error
        // Example:
        std::cerr << "Error writing to character device!" << std::endl;
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
