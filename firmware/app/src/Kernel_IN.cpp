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
    m_killThread(false),
    m_Kernel_INRx(new std::vector<char>(CHAR_DEVICE_SIZE)),
    m_Kernel_INTx(new std::vector<char>(CHAR_DEVICE_SIZE)),
    m_consoleControl(new std::vector<char>(CHAR_CONSOLE_SIZE)),
    m_Kernel_INwait(true)
{
    // Initialize m_Kernel_INRx, m_Kernel_INTx, and m_consoleControl with zeros
    std::fill(m_Kernel_INRx->begin(), m_Kernel_INRx->end(), 0);
    std::fill(m_Kernel_INTx->begin(), m_Kernel_INTx->end(), 0);
    std::fill(m_consoleControl->begin(), m_consoleControl->end(), 0);

    Info("[CONSTRUCTOR] Instantiate Kernel_IN");
}

Kernel_IN::~Kernel_IN() 
{
    Info("[DESTRUCTOR] Destroy Kernel_IN");
    if (m_Kernel_INThread.joinable()) 
    {
        m_Kernel_INThread.join();
    }

    m_Kernel_INwait = false;
}

int Kernel_IN::openDEV() 
{
    m_file_descriptor = open("/dev/Kernel_IN", O_RDWR);

    if(m_file_descriptor < 0)
    {
        Error("[Kernel_IN] Failed to open Device");
        m_killThread = true;
        return ERROR;
    } 
    else 
    {
        Info("[Kernel_IN] Device opened successfuly");
        initThread();
    }

    return OK;
}

int Kernel_IN::dataRX()
{
    int ret;

    ret = read(m_file_descriptor, m_Kernel_INRx->data(), CHAR_DEVICE_SIZE);
    
    if (ret == -1)
    {
        Error("[Kernel_IN] Cannot read from kernel space");
        return ERROR;
    }
    else if (ret == 0)
    {
        Error("[Kernel_IN] No data available");
        return ENODATA;
    }
    else
    {
        // Print received data for debugging
        Read(m_Kernel_INRx->data());

        /* Clear char device Rx buffer */
        m_Kernel_INRx->clear();

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
    (*m_Kernel_INTx)[0] = static_cast<char>(Compute::computeDeviceAddress(m_consoleControl->data()));
    (*m_Kernel_INTx)[1] = static_cast<char>(Compute::computeRegisterAddress(m_consoleControl->data()));
    (*m_Kernel_INTx)[2] = static_cast<char>(Compute::computeRegisterControl(m_consoleControl->data()));

    /* If Write then compute RegisterData */
    if((*m_Kernel_INTx)[2] == 0x01)
    {
        (*m_Kernel_INTx)[3] = static_cast<char>(Compute::computeRegisterData(m_consoleControl->data()));

        if((*m_Kernel_INTx)[0] == 0xFF || (*m_Kernel_INTx)[1] == 0xFF || (*m_Kernel_INTx)[2] == 0xFF || (*m_Kernel_INTx)[3] == 0xFF) 
        {
            Error("[Kernel_IN] Bytes computation failure [WR]");
            return ret;
        }
    }
    else
    {
        if((*m_Kernel_INTx)[0] == 0xFF || (*m_Kernel_INTx)[1] == 0xFF || (*m_Kernel_INTx)[2] == 0xFF) 
        {
            Error("[Kernel_IN] Bytes computation failure [RD]");
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
        (*m_Kernel_INTx)[3] = 0x00;
    }

#else

    /* Wait for data from TCP client */
    Info("[Kernel_IN] Wait for the Kernel_IN Flag");
    while(true == m_Kernel_INwait)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    m_Kernel_INwait = true;

#endif

    ret = write(m_file_descriptor, m_Kernel_INTx->data(), 4);

    if (ret == -1)
    {
        Error("[Kernel_IN] Cannot write to kernel space");
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
    (*m_Kernel_INTx)[0] = 0x12; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    (*m_Kernel_INTx)[1] = 0x34; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    ret = write(m_file_descriptor, m_Kernel_INTx->data(), 2);

    
    m_Kernel_INTx->clear(); /* Clear charDevice Rx buffer */
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

    m_killThread = true;
    
    return OK;
}

void Kernel_IN::initThread()
{
    Info("[THREAD] Initialize Kernel_IN");
    m_Kernel_INThread = std::thread(&Kernel_IN::Kernel_INThread, this);
}

bool Kernel_IN::isThreadKilled()
{
	return m_killThread;
}

void Kernel_IN::Kernel_INThread()
{
    while(!m_killThread) 
    {
        if(OK != dataTX())
        {
            Error("[Kernel_IN] Cannot write into the console");
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
                Error("[Kernel_IN] Cannot read from the console");
            }
#endif
            /* TODO :: Set the flag to indicate the data is ready */

            /* TODO :: Temporary */
            m_killThread = true;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Info("[THREAD] Terminate Kernel_IN");
}

void Kernel_IN::setKernel_INTx(std::vector<char>* DataRx)
{
    m_Kernel_INTx = DataRx;
    Info("[Kernel_IN] Release Kernel_IN Flag");
    m_Kernel_INwait = false;
}