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

#include "KernelInput.h"

KernelInput::KernelInput() : 
    m_file_descriptor(0), 
    m_threadKill(false),
    m_currentState(KernelInput_IDLE),
    m_Rx_KernelInput(new std::vector<char>(CHAR_DEVICE_SIZE)),
    m_Tx_KernelInput(new std::vector<char>(CHAR_DEVICE_SIZE)),
    m_consoleControl(new std::vector<char>(CHAR_CONSOLE_SIZE)),
    m_waitKernelInput(true)
{
    // Initialize m_Rx_KernelInput, m_Tx_KernelInput, and m_consoleControl with zeros
    std::fill(m_Rx_KernelInput->begin(), m_Rx_KernelInput->end(), 0);
    std::fill(m_Tx_KernelInput->begin(), m_Tx_KernelInput->end(), 0);
    std::fill(m_consoleControl->begin(), m_consoleControl->end(), 0);

    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate KernelInput" << std::endl;

}

KernelInput::~KernelInput() 
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy KernelInput" << std::endl;
    if (m_threadKernelInput.joinable()) 
    {
        m_threadKernelInput.join();
    }

    m_waitKernelInput = false;

    delete m_Rx_KernelInput;
    delete m_Tx_KernelInput;
    delete m_consoleControl;
}

int KernelInput::openDEV() 
{
    m_file_descriptor = open("/dev/KernelInput", O_RDWR);

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

int KernelInput::dataRX()
{
    int ret;

    ret = read(m_file_descriptor, m_Rx_KernelInput->data(), CHAR_DEVICE_SIZE);
    
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
        Read(m_Rx_KernelInput->data());

        /* Clear char device Rx buffer */
        m_Rx_KernelInput->clear();

        return OK;
    }
}

int KernelInput::dataTX()
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
    (*m_Tx_KernelInput)[0] = static_cast<char>(Compute::computeDeviceAddress(m_consoleControl->data()));
    (*m_Tx_KernelInput)[1] = static_cast<char>(Compute::computeRegisterAddress(m_consoleControl->data()));
    (*m_Tx_KernelInput)[2] = static_cast<char>(Compute::computeRegisterControl(m_consoleControl->data()));

    /* If Write then compute RegisterData */
    if((*m_Tx_KernelInput)[2] == 0x01)
    {
        (*m_Tx_KernelInput)[3] = static_cast<char>(Compute::computeRegisterData(m_consoleControl->data()));

        if((*m_Tx_KernelInput)[0] == 0xFF || (*m_Tx_KernelInput)[1] == 0xFF || (*m_Tx_KernelInput)[2] == 0xFF || (*m_Tx_KernelInput)[3] == 0xFF) 
        {
            Error("[IN] Bytes computation failure [WR]");
            return ret;
        }
    }
    else
    {
        if((*m_Tx_KernelInput)[0] == 0xFF || (*m_Tx_KernelInput)[1] == 0xFF || (*m_Tx_KernelInput)[2] == 0xFF) 
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
        (*m_Tx_KernelInput)[3] = 0x00;
    }

#else

    /* Wait for data from TCP client */
    Info("[IN] Wait for clear the KernelInput Flag");
    while(true == m_waitKernelInput)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    m_waitKernelInput = true;

#endif

    ret = write(m_file_descriptor, m_Tx_KernelInput->data(), 4);

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
    (*m_Tx_KernelInput)[0] = 0x12; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    (*m_Tx_KernelInput)[1] = 0x34; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    ret = write(m_file_descriptor, m_Tx_KernelInput->data(), 2);

    
    m_Tx_KernelInput->clear(); /* Clear charDevice Rx buffer */
    m_consoleControl->clear(); /* Clear console control buffer */

    return OK;
}

int KernelInput::closeDEV() 
{
    if (m_file_descriptor >= 0) 
    {
        close(m_file_descriptor);
        m_file_descriptor = -1; // Mark as closed
    }

    m_threadKill = true;
    
    return OK;
}

void KernelInput::initThread()
{
    Info("[THREAD] Initialize KernelInput");
    m_threadKernelInput = std::thread(&KernelInput::threadKernelInput, this);
}

bool KernelInput::isThreadKilled()
{
	return m_threadKill;
}

void KernelInput::threadKernelInput()
{
    while(!m_threadKill) 
    {
        switch(m_currentState)
        {
            case KernelInput_IDLE:
                break;

            case KernelInput_TX:
                std::cout << "[INFO] [KernelInput] KernelInput_TX mode" << std::endl;
                break;

            default:
                std::cout << "[INFO] [KernelInput] Unknown mode" << std::endl;
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
#if 0 /* No need for feedback here since we get it over KernelOutput */
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

    Info("[THREAD] Terminate KernelInput");
}

void KernelInput::setTx_KernelInput(std::vector<char>* DataRx)
{
    m_Tx_KernelInput = DataRx;
    Info("[IN] Release KernelInput Flag");
    m_waitKernelInput = false;
}

void KernelInput::setKernelInputState(KernelInput_stateType newState)
{
    m_currentState = newState;
}