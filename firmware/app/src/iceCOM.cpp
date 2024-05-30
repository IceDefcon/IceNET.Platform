/*!
 * 
 * Author: Ice.Marek and Ventor
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
#include <chrono>           // sleep_for
#include <thread>           // sleep_for
#include "iceCOM.h"

#define iceDEV "/dev/iceCOM"

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
    if (m_iceCOMThread.joinable()) 
    {
    	m_iceCOMThread.join();
   	}
}

void iceCOM::initThread()
{
	Console::Info("[COM] Init the iceCOMThread");
	m_iceCOMThread = std::thread(&iceCOM::iceCOMThread, this);
}

void iceCOM::iceCOMThread()
{
	Console::Info("[COM] Enter iceCOMThread");

    while(!m_killThread) 
    {
    	if(OK != dataTX())
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
	    	if(OK != dataRX())
	    	{
				Console::Error("[COM] Cannot read from the console");
	    	}
    	}

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

	Console::Info("[COM] Terminate iceCOMThread");
}

int iceCOM::openCOM() 
{
    m_file_descriptor = open(iceDEV, O_RDWR);

    if(m_file_descriptor < 0)
    {
        Console::Error("[COM] Failed to open Device");
        m_killThread = true;
        return ERROR;
    } 
    else 
    {
        Console::Info("[COM] Device opened successfuly");
        initThread();
    }

    return OK;
}

int iceCOM::dataRX()
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

uint8_t iceCOM::computeDeviceAddress(const char* in)
{
    uint8_t temp[2] = {0x00};
    uint8_t out = 0x00;

    temp[0] = in[0];
    temp[1] = in[1];

    if(temp[0] >= 0x30 && temp[0] <= 0x37)
    {
        out = (temp[0] - 0x30) << 4;
    }
    else
    {
        Console::Error("[COM] Bad device address :: Max 7-bits");
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
        Console::Error("[COM] Bad device address :: Max 7-bits");
        return 0xFF;
    }

    return out;
}

uint8_t iceCOM::computeRegisterAddress(const char* in)
{
    uint8_t temp[3] = {0x00};
    uint8_t out = 0x00;

    temp[0] = in[2];
    temp[1] = in[3];
    temp[2] = in[4];

    if(temp[0] == 0x20) /* Check for ASCII space */ 
    {
        if(temp[1] >= 0x30 && temp[1] <= 0x37)
        {
            out = (temp[1] - 0x30) << 4;
        }
        else
        {
            Console::Error("[COM] Register Not Found");
            return 0xFF;
        }

        if(temp[2] >= 0x30 && temp[2] <= 0x39)
        {
            out = out + temp[2] - 0x30;
        }
        else if(temp[2] >= 0x61 && temp[2] <= 0x66)
        {
            out = out + temp[2] - 0x61 + 0x0A;
        }
        else
        {
            Console::Error("[COM] Register Not Found");
            return 0xFF;
        }
    }
    else
    {
        Console::Error("[COM] No space between DevicAaddress & RegisterAddress");
        return 0xFF;
    }

    return out;
}

uint8_t iceCOM::computeRegisterControl(const char* in)
{
    uint8_t temp[2] = {0x00};
    uint8_t out = 0x00;

    temp[0] = in[5];
    temp[1] = in[6];

    if(temp[0] == 0x20) /* Check for ASCII space */ 
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
        Console::Error("[COM] No space between RegisterAddress & R/W operator");
        return 0xFF;
    }

    return out;
}

uint8_t iceCOM::computeRegisterData(const char* in)
{
    uint8_t temp[3] = {0x00};
    uint8_t out = 0x00;

    temp[0] = in[7];
    temp[1] = in[8];
    temp[2] = in[9];

    if(temp[0] == 0x20) /* Check for ASCII space */ 
    {
        if(temp[1] >= 0x30 && temp[1] <= 0x39)
        {
            out = (temp[1] - 0x30) << 4;
        }
        else if(temp[1] >= 0x61 && temp[1] <= 0x66)
        {
            out = (temp[1] - 0x61 + 0x0A) << 4;
        }
        else
        {
            Console::Error("[COM] Invalid Write Data");
            return 0xFF;
        }

        if(temp[2] >= 0x30 && temp[2] <= 0x39)
        {
            out = out + temp[2] - 0x30;
        }
        else if(temp[2] >= 0x61 && temp[2] <= 0x66)
        {
            out = out + temp[2] - 0x61 + 0x0A;
        }
        else
        {
            Console::Error("[COM] Invalid Write Data");
            return 0xFF;
        }
    }
    else
    {
        Console::Error("[COM] No space between R/W operator & RegisterData");
        return 0xFF;
    }

    return out;
}

#include "iceNET.h"

int iceCOM::dataTX()
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
#if 0 /* init server from i2c cosole */
    else if (std::strcmp(consoleControl.data(), "tcp") == 0)
    {
        Console::Info("[NET] Creating iceNET object");
        iceNET* iceNETServer = new iceNET(2555); /* tcp Server */
        iceNETServer->openCOM("tcpServer");
    }
#endif
#if 1 /* Read Enable in FIFO */
    else if (std::strcmp(consoleControl.data(), "rd") == 0)
    {
        charDeviceTx[0] = 0x12; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
        charDeviceTx[1] = 0x34; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
        ret = write(m_file_descriptor, charDeviceTx.data(), 2);
        return ret;
    }
#endif

    /**
     * 
     * We have to pass data trough the FIFO
     * to separate bytes from eachother 
     * in order to pass them to the 
     * i2c state machine in FPGA
     * 
     * Byte[0] :: Device Address
     * Byte[1] :: Register Address
     * Byte[2] :: Register Control
     * Byte[3] :: Register Data
     * 
     */
    charDeviceTx[0] = computeDeviceAddress(consoleControl.data());
    charDeviceTx[1] = computeRegisterAddress(consoleControl.data());
    charDeviceTx[2] = computeRegisterControl(consoleControl.data());

    /* If Write then compute RegisterData */
    if(charDeviceTx[2] == 0x01)
    {
        charDeviceTx[3] = computeRegisterData(consoleControl.data());

        if(charDeviceTx[0] == 0xFF || charDeviceTx[1] == 0xFF || charDeviceTx[2] == 0xFF || charDeviceTx[3] == 0xFF) 
        {
            Console::Error("[COM] Bytes computation failure [WR]");
            return ret;
        }
    }
    else
    {
        if(charDeviceTx[0] == 0xFF || charDeviceTx[1] == 0xFF || charDeviceTx[2] == 0xFF) 
        {
            Console::Error("[COM] Bytes computation failure [RD]");
            return ret;
        }
        
        /**
         * 
         * Additional byte 
         * at read procedure
         * to always make 4 bytes 
         * FIFO input/output geometry
         * 
         */
        charDeviceTx[3] = 0x00;
    }

    ret = write(m_file_descriptor, charDeviceTx.data(), 4);

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

int iceCOM::closeCOM() 
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
