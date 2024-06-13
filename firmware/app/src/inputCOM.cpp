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

#include "inputCOM.h"

inputCOM::inputCOM() : 
    m_file_descriptor(0), 
    m_killThread(false),
    m_inputCOMRx(new std::vector<char>(CHAR_DEVICE_SIZE)),
    m_inputCOMTx(new std::vector<char>(CHAR_DEVICE_SIZE)),
    m_consoleControl(new std::vector<char>(CHAR_CONSOLE_SIZE)),
    m_inputCOMwait(true)
{
    // Initialize m_inputCOMRx, m_inputCOMTx, and m_consoleControl with zeros
    std::fill(m_inputCOMRx->begin(), m_inputCOMRx->end(), 0);
    std::fill(m_inputCOMTx->begin(), m_inputCOMTx->end(), 0);
    std::fill(m_consoleControl->begin(), m_consoleControl->end(), 0);

    Info("[CONSTRUCTOR] Instantiate inputCOM");
}

inputCOM::~inputCOM() 
{
    Info("[DESTRUCTOR] Destroy inputCOM");
    if (m_inputCOMThread.joinable()) 
    {
        m_inputCOMThread.join();
    }

    m_inputCOMwait = false;
}

int inputCOM::openDEV() 
{
    m_file_descriptor = open("/dev/inputCOM", O_RDWR);

    if(m_file_descriptor < 0)
    {
        Error("[inputCOM] Failed to open Device");
        m_killThread = true;
        return ERROR;
    } 
    else 
    {
        Info("[inputCOM] Device opened successfuly");
        initThread();
    }

    return OK;
}

int inputCOM::dataRX()
{
    int ret;

    ret = read(m_file_descriptor, m_inputCOMRx->data(), CHAR_DEVICE_SIZE);
    
    if (ret == -1)
    {
        Error("[inputCOM] Cannot read from kernel space");
        return ERROR;
    }
    else if (ret == 0)
    {
        Error("[inputCOM] No data available");
        return ENODATA;
    }
    else
    {
        // Print received data for debugging
        Read(m_inputCOMRx->data());

        /* Clear char device Rx buffer */
        m_inputCOMRx->clear();

        return OK;
    }
}

int inputCOM::dataTX()
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
    (*m_inputCOMTx)[0] = static_cast<char>(Compute::computeDeviceAddress(m_consoleControl->data()));
    (*m_inputCOMTx)[1] = static_cast<char>(Compute::computeRegisterAddress(m_consoleControl->data()));
    (*m_inputCOMTx)[2] = static_cast<char>(Compute::computeRegisterControl(m_consoleControl->data()));

    /* If Write then compute RegisterData */
    if((*m_inputCOMTx)[2] == 0x01)
    {
        (*m_inputCOMTx)[3] = static_cast<char>(Compute::computeRegisterData(m_consoleControl->data()));

        if((*m_inputCOMTx)[0] == 0xFF || (*m_inputCOMTx)[1] == 0xFF || (*m_inputCOMTx)[2] == 0xFF || (*m_inputCOMTx)[3] == 0xFF) 
        {
            Error("[inputCOM] Bytes computation failure [WR]");
            return ret;
        }
    }
    else
    {
        if((*m_inputCOMTx)[0] == 0xFF || (*m_inputCOMTx)[1] == 0xFF || (*m_inputCOMTx)[2] == 0xFF) 
        {
            Error("[inputCOM] Bytes computation failure [RD]");
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
        (*m_inputCOMTx)[3] = 0x00;
    }

#else

    /* Wait for data from TCP client */
    Info("[inputCOM] Wait for the inputCOM Flag");
    while(true == m_inputCOMwait)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    m_inputCOMwait = true;

#endif

    ret = write(m_file_descriptor, m_inputCOMTx->data(), 4);

    if (ret == -1)
    {
        Error("[inputCOM] Cannot write to kernel space");
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
    (*m_inputCOMTx)[0] = 0x12; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    (*m_inputCOMTx)[1] = 0x34; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    ret = write(m_file_descriptor, m_inputCOMTx->data(), 2);

    
    m_inputCOMTx->clear(); /* Clear charDevice Rx buffer */
    m_consoleControl->clear(); /* Clear console control buffer */

    return OK;
}

int inputCOM::closeDEV() 
{
    if (m_file_descriptor >= 0) 
    {
        close(m_file_descriptor);
        m_file_descriptor = -1; // Mark as closed
    }

    m_killThread = true;
    
    return OK;
}

void inputCOM::initThread()
{
    Info("[THREAD] Initialize inputCOM");
    m_inputCOMThread = std::thread(&inputCOM::inputCOMThread, this);
}

bool inputCOM::isThreadKilled()
{
	return m_killThread;
}

void inputCOM::inputCOMThread()
{
    while(!m_killThread) 
    {
        if(OK != dataTX())
        {
            Error("[inputCOM] Cannot write into the console");
        }
        else
        {
            /**
             * 
             * At the moment Feedback only print info 
             * about successfully transfered command 
             * 
             */
#if 0 /* No need for feedback here since we get it over outputCOM */
            if(OK != dataRX())
            {
                Error("[inputCOM] Cannot read from the console");
            }
#endif
            /* TODO :: Set the flag to indicate the data is ready */

            /* TODO :: Temporary */
            m_killThread = true;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Info("[THREAD] Terminate inputCOM");
}

void inputCOM::setinputCOMTx(std::vector<char>* DataRx)
{
    m_inputCOMTx = DataRx;
    Info("[inputCOM] Release inputCOM Flag");
    m_inputCOMwait = false;
}