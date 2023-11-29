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

    // struct termios old_tio, new_tio;

    // // Get the terminal settings
    // tcgetattr(STDIN_FILENO, &old_tio);

    // new_tio = old_tio;
    // new_tio.c_lflag &= (~ICANON & ~ECHO); // Disable canonical mode and echoing

    // // Set the new terminal settings
    // tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

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

    // // Restore the original terminal settings
    // tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    
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

int iceCOM::device_write()
{
	int ret;
    char ch;

	Debug::Write();

    while (1) {
        // Read a single character without Enter
        ch = getchar();
        
        // Check if the pressed key is 'q'
        if (ch == 'q') 
        {
            m_killThread = true;
            break;
        }
        if (ch == 'u') break;
        if (ch == 'd') break;
        if (ch == 'l') break;
        if (ch == 'r') break;
    }

	ret = write(m_file_descriptor, &ch, 1); // Send the string to the LKM
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
