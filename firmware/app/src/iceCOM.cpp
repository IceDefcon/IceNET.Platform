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
#include <vector>
#include <iomanip>          // for std::hex and std::setfill

#include "iceCOM.h"

iceCOM::iceCOM() : 
    m_file_descriptor(0), 
    m_killThread(false),
    m_iceCOMRx(CHAR_DEVICE_SIZE),
    m_iceCOMTx(CHAR_DEVICE_SIZE),
    m_consoleControl(CHAR_CONSOLE_SIZE)
{
    // Initialize m_iceCOMRx, m_iceCOMTx, and m_consoleControl with zeros
    std::fill(m_iceCOMRx.begin(), m_iceCOMRx.end(), 0);
    std::fill(m_iceCOMTx.begin(), m_iceCOMTx.end(), 0);
    std::fill(m_consoleControl.begin(), m_consoleControl.end(), 0);

    Info("[CONSTRUCTOR] Instantiate iceCOM");
}

iceCOM::~iceCOM() 
{
    Info("[DESTRUCTOR] Destroy iceCOM");
    if (m_iceCOMThread.joinable()) 
    {
        m_iceCOMThread.join();
    }
}

int iceCOM::openDEV() 
{
    m_file_descriptor = open("/dev/iceCOM", O_RDWR);

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
    ret = read(m_file_descriptor, m_iceCOMRx.data(), CHAR_DEVICE_SIZE);
    
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
        Read(m_iceCOMRx.data());

        /* Clear char device Rx buffer */
        m_iceCOMRx.clear();

        return OK;
    }
}

int iceCOM::dataTX()
{
    int ret = -1;

    Write();
    /* Get console characters */
    std::cin.getline(m_consoleControl.data(), CHAR_CONSOLE_SIZE);

    if (std::strcmp(m_consoleControl.data(), "exit") == 0) 
    {
        m_killThread = true;
        return ret;
    }

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
    m_iceCOMTx[0] = Compute::computeDeviceAddress(m_consoleControl.data());
    m_iceCOMTx[1] = Compute::computeRegisterAddress(m_consoleControl.data());
    m_iceCOMTx[2] = Compute::computeRegisterControl(m_consoleControl.data());

    /* If Write then compute RegisterData */
    if(m_iceCOMTx[2] == 0x01)
    {
        m_iceCOMTx[3] = Compute::computeRegisterData(m_consoleControl.data());

        if(m_iceCOMTx[0] == 0xFF || m_iceCOMTx[1] == 0xFF || m_iceCOMTx[2] == 0xFF || m_iceCOMTx[3] == 0xFF) 
        {
            Error("[COM] Bytes computation failure [WR]");
            return ret;
        }
    }
    else
    {
        if(m_iceCOMTx[0] == 0xFF || m_iceCOMTx[1] == 0xFF || m_iceCOMTx[2] == 0xFF) 
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
        m_iceCOMTx[3] = 0x00;
    }

    ret = write(m_file_descriptor, m_iceCOMTx.data(), 4);

    if (ret == -1)
    {
        Error("[COM] Cannot write to kernel space");
        return ERROR;
    }

    /*!
     * 
     * Offload FIFO signal require at least 
     * 14ms for the for the I2C state 
     * machine to process transfer
     * 
     * TODO :: RTL state machine for I2C operation
     * can be optimized due to the delay between 
     * the transitions of states
     * 
     * 20ms is used for data ransfer safety
     * 
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    m_iceCOMTx[0] = 0x12; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    m_iceCOMTx[1] = 0x34; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    ret = write(m_file_descriptor, m_iceCOMTx.data(), 2);

    
    m_iceCOMTx.clear(); /* Clear charDevice Rx buffer */
    m_consoleControl.clear(); /* Clear console control buffer */

    return OK;
}

int iceCOM::closeDEV() 
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

        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Info("[COM] Terminate iceCOMThread");
}
