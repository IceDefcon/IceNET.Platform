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
    m_waitKernelInput(true)
{
    // Initialize m_Rx_KernelInput and m_Tx_KernelInput with zeros
    std::fill(m_Rx_KernelInput->begin(), m_Rx_KernelInput->end(), 0);
    std::fill(m_Tx_KernelInput->begin(), m_Tx_KernelInput->end(), 0);

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
}

int KernelInput::openDEV() 
{
    m_file_descriptor = open("/dev/KernelInput", O_RDWR);

    if(m_file_descriptor < 0)
    {
        Error("[ I ] Failed to open Device");
        m_threadKill = true;
        return ERROR;
    } 
    else 
    {
        Info("[ I ] Device opened successfuly");
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
        Error("[ I ] Cannot read from kernel space");
        return ERROR;
    }
    else if (ret == 0)
    {
        Error("[ I ] No data available");
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

    /* Wait for data from TCP client */
    Info("[ I ] Wait untile waitKernelInput flag is released");
    while(true == m_waitKernelInput)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    m_waitKernelInput = true;

    ret = write(m_file_descriptor, m_Tx_KernelInput->data(), 4);

    if (ret == -1)
    {
        Error("[ I ] Cannot write to kernel space");
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
     * 20ms is used for data transfer safety
     * 
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    (*m_Tx_KernelInput)[0] = 0x12; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    (*m_Tx_KernelInput)[1] = 0x34; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    ret = write(m_file_descriptor, m_Tx_KernelInput->data(), 2);

    
    m_Tx_KernelInput->clear(); /* Clear charDevice Rx buffer */

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
    Info("[ I ] Initialize threadKernelInput");
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
                std::cout << "[INFO] [ I ] KernelInput_TX mode" << std::endl;

                if(OK != dataTX())
                {
                    Error("[ I ] Cannot write into the Kernel Space");
                }
                else
                {
                    Error("[ I ] Data successfuly written into the Kernel Space");
                }

                setKernelInputState(KernelInput_IDLE);
                break;

            case KernelInput_KILL:
                std::cout << "[INFO] [ I ] set KernelInput_KILL mode" << std::endl;
                (*m_Tx_KernelInput)[0] = 0xDE;
                (*m_Tx_KernelInput)[1] = 0xAD;
                write(m_file_descriptor, m_Tx_KernelInput->data(), 2);
                m_Tx_KernelInput->clear();
                setKernelInputState(KernelInput_IDLE);
                break;

            default:
                std::cout << "[INFO] [ I ] Unknown mode" << std::endl;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Info("[ I ] Terminate threadKernelInput");
}

void KernelInput::setTx_KernelInput(std::vector<char>* DataRx)
{
    m_Tx_KernelInput = DataRx;
    Info("[ I ] Release waitKernelInput flag");
    m_waitKernelInput = false;
}

void KernelInput::setKernelInputState(KernelInput_stateType newState)
{
    m_currentState = newState;
}