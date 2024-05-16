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

uint8_t iceCOM::computeRegisterAddress(const char* in)
{
    uint8_t temp[2] = {0};
    uint8_t out = 0;

    temp[0] = in[0];
    temp[1] = in[1];

    if(temp[0] >= 0x30 && temp[0] <= 0x37)
    {
        out = (temp[0] - 0x30) << 4;
    }
    else
    {
        Console::Error("[COM] Register Not Found");
        return 0xFF;
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
        Console::Error("[COM] Register Not Found");
        return 0xFF;
    }

    return out;
}

uint8_t iceCOM::computeControlRegister(const char* in)
{
    uint8_t temp[2] = {0};
    uint8_t out = 0;

    temp[0] = in[2];
    temp[1] = in[3];

    if(temp[0] == 0x20)
    {
        if(temp[1] == 0x72) out = 0x00; /* Read */
        else if(temp[1] == 0x77) out = 0x01; /* Write */
        else
        {
            Console::Error("[COM] Bad R/W operator");
            return 0xFF;
        }
    }
    else
    {
        Console::Error("[COM] No space between register and R/W operator");
        return 0xFF;
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
        return ret;
    }

#if 1 /* Read Enable in FIFO */
    else if (std::strcmp(consoleControl.data(), "rd") == 0)
    {
        charDeviceTx[0] = 0x12; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
        charDeviceTx[1] = 0x34; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
        ret = write(m_file_descriptor, charDeviceTx.data(), 2);
        return ret;
    }

    charDeviceTx[0] = computeRegisterAddress(consoleControl.data());
#endif

#if 0 /* Register + Control Byte */
    /**
     * 
     * TODO
     * 
     * Control Register coputation is ready
     * However, we need to pass data trough
     * the FIFO in order to separate bytes
     * from eachother in order to pass
     * them to i2c state machine
     * 
     * 1. BMI160 register
     * 2. Control Byte
     * 
     */

    charDeviceTx[1] = computeControlRegister(consoleControl.data());

    if(charDeviceTx[0] == 0xFF || charDeviceTx[1] == 0xFF) 
    {
        Console::Error("[COM] Bytes computation failure");
        return ret;
    }

    ret = write(m_file_descriptor, charDeviceTx.data(), 2);
#else /* Just Register Byte */
    ret = write(m_file_descriptor, charDeviceTx.data(), 1);
#endif

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
