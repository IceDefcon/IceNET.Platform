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
consoleControl(CHAR_CONSOLE_SIZE),
m_dataReady(false)
{
    /* Initialize charDeviceRx, charDeviceTx, and consoleControl with zeros */
    std::fill(charDeviceRx.begin(), charDeviceRx.end(), 0);
    std::fill(charDeviceTx.begin(), charDeviceTx.end(), 0);
    std::fill(consoleControl.begin(), consoleControl.end(), 0);

    Info("[CONSTRUCTOR] Initialise iceCOM Object");
}

iceCOM::~iceCOM() 
{
	Info("[DESTRUCTOR] Destroy iceCOM Object");
    if (m_iceCOMThread.joinable()) 
    {
    	m_iceCOMThread.join();
   	}
}

int iceCOM::openCOM() 
{
    m_file_descriptor = open(iceDEV, O_RDWR);

    if(m_file_descriptor < 0)
    {
        Error("[COM] Failed to open Device");
        m_killThread = true;
        return ERROR;
    } 
    else 
    {
        Info("[COM] Device opened successfuly");
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
        Error("[COM] Cannot read from kernel space");
        return ERROR;
    }
    else if (ret == 0)
    {
        Error("[COM] No data available");
        return ENODATA;
    }
    else
    {
        // Print received data for debugging
        Read(charDeviceRx.data());

        /* Clear char device Rx buffer */
        charDeviceRx.clear();

        return OK;
    }
}

int iceCOM::dataTX()
{
    int ret = -1;

    Write();
    /* Get console characters */
    std::cin.getline(consoleControl.data(), CHAR_CONSOLE_SIZE);

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
    charDeviceTx[0] = Compute::computeDeviceAddress(consoleControl.data());
    charDeviceTx[1] = Compute::computeRegisterAddress(consoleControl.data());
    charDeviceTx[2] = Compute::computeRegisterControl(consoleControl.data());

    /* If Write then compute RegisterData */
    if(charDeviceTx[2] == 0x01)
    {
        charDeviceTx[3] = Compute::computeRegisterData(consoleControl.data());

        if(charDeviceTx[0] == 0xFF || charDeviceTx[1] == 0xFF || charDeviceTx[2] == 0xFF || charDeviceTx[3] == 0xFF) 
        {
            Error("[COM] Bytes computation failure [WR]");
            return ret;
        }
    }
    else
    {
        if(charDeviceTx[0] == 0xFF || charDeviceTx[1] == 0xFF || charDeviceTx[2] == 0xFF) 
        {
            Error("[COM] Bytes computation failure [RD]");
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
        Error("[COM] Cannot write to kernel space");
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

void iceCOM::initThread()
{
    Info("[COM] Init the iceCOMThread");
    m_iceCOMThread = std::thread(&iceCOM::iceCOMThread, this);
}

bool iceCOM::isThreadKilled()
{
	return m_killThread;
}

void iceCOM::iceCOMThread()
{
    Info("[COM] Enter iceCOMThread");

    while(!m_killThread) 
    {
        if(OK != dataTX())
        {
            Error("[COM] Cannot write into the console");
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
                Error("[COM] Cannot read from the console");
            }

            /* TODO :: Set the flag to indicate the data is ready */
            setDataReady(true);
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Info("[COM] Terminate iceCOMThread");
}

bool iceCOM::getDataReady()
{
    return m_dataReady;
}

void iceCOM::setDataReady(bool flag)
{
    m_dataReady = flag;
}
