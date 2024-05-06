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
#include <cstdint>          // for uint8_t
#include "iceCOM.h"


iceCOM::iceCOM(): 
m_file_descriptor(0), 
m_killThread(false),
charDeviceRx(CHAR_DEVICE_SIZE),
charDeviceTx(CHAR_DEVICE_SIZE),
consoleControl(CONSOLE_CONTROL_SIZE)
{
    /* Initialize charDeviceRx, charDeviceTx, and consoleControl with zeros */
    std::fill(charDeviceRx.begin(), charDeviceRx.end(), 0);
    std::fill(charDeviceTx.begin(), charDeviceTx.end(), 0);
    std::fill(consoleControl.begin(), consoleControl.end(), 0);

    Console::Info("[COM] Initialise iceCOM Module");
}

iceCOM::~iceCOM() 
{
	Console::Info("[COM] Destroying iceCOM Module");
    if (m_iceThread.joinable()) 
    {
    	m_iceThread.join();
   	}
}

void iceCOM::initThread()
{
	Console::Info("[COM] Init the iceCOMThread");
	m_iceThread = std::thread(&iceCOM::iceCOMThread, this);
}

void iceCOM::iceCOMThread()
{
	Console::Info("[COM] Enter iceCOMThread");

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
			Console::Error("[COM] Cannot write into the console");
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
				Console::Error("[COM] Cannot read from the console");
	    	}
    	}
    }

	Console::Info("[COM] Terminate iceCOMThread");
}

int iceCOM::device_open(const char* device) 
{
    m_file_descriptor = open(device, O_RDWR);

    if(m_file_descriptor < 0)
    {
        Console::Error("[COM] Failed to open Device");
        m_killThread = true;
        return ERROR;
    } else 
    {
        Console::Info("[COM] Device opened successfuly");
        initThread();
    }

    return OK;
}


int iceCOM::device_read()
{
    int ret;

    // Attempt to read data from kernel space
    ret = read(m_file_descriptor, charDeviceRx.data(), CHAR_DEVICE_SIZE);
    if (ret == -1)
    {
        Console::Error("[COM] Cannot read from kernel space");
        return ERROR;
    }
    else if (ret == 0)
    {
        Console::Error("[COM] No data available");
        return ENODATA;
    }
    else
    {
        // Print received data for debugging
        Console::Read(charDeviceRx.data());

        /* Clear char device Rx buffer */
        charDeviceRx.clear();

        return OK;
    }
}



uint8_t computeRegister(const char* in)
{
    uint8_t temp[2] = 0;
    uint8_t out = 0;

    temp[0] = in[0];
    temp[1] = in[1];

    if(temp[0] >= 0x30 && temp[0] <= 0x37)
    {
        out = (temp[0] - 0x30) << 4;
    }
    else
    {
        out = 0xFF;
        Console::Error("[COM] Register Not Found");
    }

    if(temp[1] >= 0x30 && temp[1] <= 0x39)
    {
        out = out + temp[1] - 0x30;
    }
    else if(temp[1] >= 0x61 && temp[1] <= 0x66)
    {
        out = out + temp[1] - 0x61 + 0x0A;
    }
    else
    {
        out = 0xFF;
        Console::Error("[COM] Register Not Found");
    }


    return out;
}


int iceCOM::device_write()
{
    int ret = -1;

    Console::Write();
    /* Get console characters */
    std::cin.getline(consoleControl.data(), CONSOLE_CONTROL_SIZE);

    if (std::strcmp(consoleControl.data(), "exit") == 0) 
    {
        m_killThread = true;
    }

    uint8_t test[2] = {0};
    uint8_t final = 0;

    test[0] = consoleControl[0];
    test[1] = consoleControl[1];

    for (int i = 0; i < 2; ++i)
    {
        printf("%x\n", test[i]);
    }

    if(test[0] >= 0x30 && test[0] <= 0x37)
    {
        final = (test[0] - 0x30) << 4;
    }
    else
    {
        final = 0xFF;
        Console::Error("[COM] Register Not Found");
    }

    if(test[1] >= 0x30 && test[1] <= 0x39)
    {
        final = final + test[1] - 0x30;
    }
    else if(test[1] >= 0x61 && test[1] <= 0x66)
    {
        final = final + test[1] - 0x61 + 0x0A;
    }
    else
    {
        final = 0xFF;
        Console::Error("[COM] Register Not Found");
    }

    printf("final 1: %x\n", final);

    final = computeRegister(consoleControl.data());

    printf("final 2: %x\n", final);

    // charDeviceTx[0] = final; /* chip id */
    // ret = write(m_file_descriptor, charDeviceTx.data(), 1);

    // if (std::strcmp(consoleControl.data(), "exit") == 0) 
    // {
    //     m_killThread = true;
    // }
    // else if (std::strcmp(consoleControl.data(), "id") == 0)
    // {
    //     charDeviceTx[0] = 0x00; /* chip id */
    //     ret = write(m_file_descriptor, charDeviceTx.data(), 1);
    // }
    // else if (std::strcmp(consoleControl.data(), "s1") == 0)
    // {
    //     charDeviceTx[0] = 0x18; /* SENSORTIME_0 */
    //     ret = write(m_file_descriptor, charDeviceTx.data(), 1);
    // }
    // else if (std::strcmp(consoleControl.data(), "s2") == 0)
    // {
    //     charDeviceTx[0] = 0x19; /* SENSORTIME_1 */
    //     ret = write(m_file_descriptor, charDeviceTx.data(), 1);
    // }
    // else if (std::strcmp(consoleControl.data(), "s3") == 0)
    // {
    //     charDeviceTx[0] = 0x1A; /* SENSORTIME_2 */
    //     ret = write(m_file_descriptor, charDeviceTx.data(), 1);
    // }
    // else if (std::strcmp(consoleControl.data(), "st") == 0)
    // {
    //     charDeviceTx[0] = 0x1B; /* status register */
    //     ret = write(m_file_descriptor, charDeviceTx.data(), 1);
    // }
    /**
     * 
     * TODO
     * 
     * Extra consideration must be taken
     * when sending data to kernel and FPGA
     * 
     * Multiple bytes must be processed sequentially
     * in order to receive multiple readings 
     * from variables and registers
     * 
     */
    // else if (std::strcmp(consoleControl.data(), "test") == 0) 
    // {
    //     charDeviceTx[0] = 0x18; /* SENSORTIME_0 */
    //     charDeviceTx[1] = 0x19; /* SENSORTIME_1 */
    //     charDeviceTx[2] = 0x1A; /* SENSORTIME_2 */
    //     ret = write(m_file_descriptor, charDeviceTx.data(), 3);
    // }
    // else
    // {
    //     Console::Error("[COM] Command not found");
    //     ret = -1;
    // }

    if (ret == -1)
    {
        Console::Error("[COM] Cannot write to kernel space");
        return ERROR;
    }

    /* Clear charDevice Rx buffer */
    charDeviceTx.clear();
    /* Clear console control buffer */
    consoleControl.clear();

    return OK;
}

int iceCOM::device_close() 
{
    if (m_file_descriptor >= 0) 
    {
        close(m_file_descriptor);
        m_file_descriptor = -1; // Mark as closed
    }

    return OK;
}

bool iceCOM::terminate()
{
	return m_killThread;
}
