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

#include "Kernel_IN.h"

Kernel_IN::Kernel_IN() : 
    m_file_descriptor(0), 
    m_threadKill(false),
    m_currentState(Kernel_IN_IDLE),
    m_Rx_Kernel_IN(new std::vector<char>(CHAR_DEVICE_SIZE)),
    m_Tx_Kernel_IN(new std::vector<char>(CHAR_DEVICE_SIZE)),
    m_consoleControl(new std::vector<char>(CHAR_CONSOLE_SIZE)),
    m_waitKernel_IN(true)
{
    // Initialize m_Rx_Kernel_IN, m_Tx_Kernel_IN, and m_consoleControl with zeros
    std::fill(m_Rx_Kernel_IN->begin(), m_Rx_Kernel_IN->end(), 0);
    std::fill(m_Tx_Kernel_IN->begin(), m_Tx_Kernel_IN->end(), 0);
    std::fill(m_consoleControl->begin(), m_consoleControl->end(), 0);

    Info("[CONSTRUCTOR] Instantiate Kernel_IN");
}

Kernel_IN::~Kernel_IN() 
{
    Info("[DESTRUCTOR] Destroy Kernel_IN");
    if (m_threadKernel_IN.joinable()) 
    {
        m_threadKernel_IN.join();
    }

    m_waitKernel_IN = false;

    delete m_Rx_Kernel_IN;
    delete m_Tx_Kernel_IN;
    delete m_consoleControl;
}

int Kernel_IN::openDEV() 
{
    m_file_descriptor = open("/dev/Kernel_IN", O_RDWR);

    if(m_file_descriptor < 0)
    {
        Error("[IN] Failed to open Device");
        m_threadKill = true;
        return ERROR;
    } 
    else 
    {
        Info("[IN] Device opened successfuly");
        initThread();
    }

    return OK;
}

int Kernel_IN::dataRX()
{
    int ret;

    ret = read(m_file_descriptor, m_Rx_Kernel_IN->data(), CHAR_DEVICE_SIZE);
    
    if (ret == -1)
    {
        Error("[IN] Cannot read from kernel space");
        return ERROR;
    }
    else if (ret == 0)
    {
        Error("[IN] No data available");
        return ENODATA;
    }
    else
    {
        // Print received data for debugging
        Read(m_Rx_Kernel_IN->data());

        /* Clear char device Rx buffer */
        m_Rx_Kernel_IN->clear();

        return OK;
    }
}

int Kernel_IN::dataTX()
{
    int ret = -1;

#if 0 /* Console control :: Moved to TCP server */

    Write();
    /* Get console characters */
    std::cin.getline(m_consoleControl->data(), CHAR_CONSOLE_SIZE);

    if (std::strcmp(m_consoleControl->data(), "exit") == 0) 
    {
        m_threadKill = true;
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
    (*m_Tx_Kernel_IN)[0] = static_cast<char>(Compute::computeDeviceAddress(m_consoleControl->data()));
    (*m_Tx_Kernel_IN)[1] = static_cast<char>(Compute::computeRegisterAddress(m_consoleControl->data()));
    (*m_Tx_Kernel_IN)[2] = static_cast<char>(Compute::computeRegisterControl(m_consoleControl->data()));

    /* If Write then compute RegisterData */
    if((*m_Tx_Kernel_IN)[2] == 0x01)
    {
        (*m_Tx_Kernel_IN)[3] = static_cast<char>(Compute::computeRegisterData(m_consoleControl->data()));

        if((*m_Tx_Kernel_IN)[0] == 0xFF || (*m_Tx_Kernel_IN)[1] == 0xFF || (*m_Tx_Kernel_IN)[2] == 0xFF || (*m_Tx_Kernel_IN)[3] == 0xFF) 
        {
            Error("[IN] Bytes computation failure [WR]");
            return ret;
        }
    }
    else
    {
        if((*m_Tx_Kernel_IN)[0] == 0xFF || (*m_Tx_Kernel_IN)[1] == 0xFF || (*m_Tx_Kernel_IN)[2] == 0xFF) 
        {
            Error("[IN] Bytes computation failure [RD]");
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
        (*m_Tx_Kernel_IN)[3] = 0x00;
    }

#else

    /* Wait for data from TCP client */
    Info("[IN] Wait for clear the Kernel_IN Flag");
    while(true == m_waitKernel_IN)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    m_waitKernel_IN = true;

#endif

    ret = write(m_file_descriptor, m_Tx_Kernel_IN->data(), 4);

    if (ret == -1)
    {
        Error("[IN] Cannot write to kernel space");
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
    (*m_Tx_Kernel_IN)[0] = 0x12; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    (*m_Tx_Kernel_IN)[1] = 0x34; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    ret = write(m_file_descriptor, m_Tx_Kernel_IN->data(), 2);

    
    m_Tx_Kernel_IN->clear(); /* Clear charDevice Rx buffer */
    m_consoleControl->clear(); /* Clear console control buffer */

    return OK;
}

int Kernel_IN::closeDEV() 
{
    if (m_file_descriptor >= 0) 
    {
        close(m_file_descriptor);
        m_file_descriptor = -1; // Mark as closed
    }

    m_threadKill = true;
    
    return OK;
}

void Kernel_IN::initThread()
{
    Info("[THREAD] Initialize Kernel_IN");
    m_threadKernel_IN = std::thread(&Kernel_IN::threadKernel_IN, this);
}

bool Kernel_IN::isThreadKilled()
{
	return m_threadKill;
}

void Kernel_IN::threadKernel_IN()
{
    while(!m_threadKill) 
    {
        switch(m_currentState)
        {
            case Kernel_IN_IDLE:
                break;

            case Kernel_IN_TX:
                std::cout << "[INFO] [Kernel_IN] Kernel_IN_TX mode" << std::endl;
                break;

            default:
                std::cout << "[INFO] [Kernel_IN] Unknown mode" << std::endl;
        }


        if(OK != dataTX())
        {
            Error("[IN] Cannot write into the console");
        }
        else
        {
            /**
             * 
             * At the moment Feedback only print info 
             * about successfully transfered command 
             * 
             */
#if 0 /* No need for feedback here since we get it over Kernel_OUT */
            if(OK != dataRX())
            {
                Error("[IN] Cannot read from the console");
            }
#endif
            /* TODO :: Set the flag to indicate the data is ready */

            /* TODO :: Temporary */
            m_threadKill = true;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Info("[THREAD] Terminate Kernel_IN");
}

void Kernel_IN::setTx_Kernel_IN(std::vector<char>* DataRx)
{
    m_Tx_Kernel_IN = DataRx;
    Info("[IN] Release Kernel_IN Flag");
    m_waitKernel_IN = false;
}

void Kernel_IN::setKernel_INState(Kernel_IN_stateType newState)
{
    m_currentState = newState;
}