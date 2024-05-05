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
m_killThread(false),
console_RX(BUFFER_LENGTH),
console_TX(BUFFER_LENGTH)
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

    // Attempt to read data from kernel space
    ret = read(m_file_descriptor, console_RX.data(), BUFFER_LENGTH);
    if (ret == -1)
    {
        Debug::Error("[iceCOM] Cannot read from kernel space");
        return ERROR;
    }
    else if (ret == 0)
    {
        Debug::Error("[iceCOM] No data available");
        return ENODATA;
    }
    else
    {
        // Print received data for debugging
        Debug::Read(console_RX.data());

        // Print the first four bytes received in hexadecimal format
        for (int i = 0; i < 4; ++i) {
            printf("Received Byte[%d]: 0x%02X\n", i, console_RX[i]);
        }

        // Clear the buffer for further reads
        console_RX.clear();
        return OK;
    }
}

int iceCOM::device_write()
{
    int ret = -1;

    Debug::Write();
    /* Get console characters */
    std::cin.getline(console_TX.data(), BUFFER_LENGTH);

    if (std::strcmp(console_TX.data(), "exit") == 0) 
    {
        m_killThread = true;
    }
    else if (std::strcmp(console_TX.data(), "id") == 0)
    {
        console_TX[0] = 0x00; /* chip id */
        ret = write(m_file_descriptor, console_TX.data(), 1);
    }
    else if (std::strcmp(console_TX.data(), "s1") == 0)
    {
        console_TX[0] = 0x18; /* SENSORTIME_0 */
        ret = write(m_file_descriptor, console_TX.data(), 1);
    }
    else if (std::strcmp(console_TX.data(), "s2") == 0)
    {
        console_TX[0] = 0x19; /* SENSORTIME_1 */
        ret = write(m_file_descriptor, console_TX.data(), 1);
    }
    else if (std::strcmp(console_TX.data(), "s3") == 0)
    {
        console_TX[0] = 0x1A; /* SENSORTIME_2 */
        ret = write(m_file_descriptor, console_TX.data(), 1);
    }
    else if (std::strcmp(console_TX.data(), "st") == 0)
    {
        console_TX[0] = 0x1B; /* status register */
        ret = write(m_file_descriptor, console_TX.data(), 1);
    }
    /**
     * 
     * TODO
     * 
     * This need to be considered when send to:
     * 1. Kernel space
     * 2. FPGA
     * 
     * Multiple bytes must be processed sequentially
     * in order to receive multiple readings from registers
     * 
     */
    else if (std::strcmp(console_TX.data(), "test") == 0) 
    {
        console_TX[0] = 0x18; /* SENSORTIME_0 */
        console_TX[1] = 0x19; /* SENSORTIME_1 */
        console_TX[2] = 0x1A; /* SENSORTIME_2 */
        ret = write(m_file_descriptor, console_TX.data(), 3);
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
