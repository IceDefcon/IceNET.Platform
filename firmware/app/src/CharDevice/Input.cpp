/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
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

#include "Input.h"

Input::Input() :
    m_file_descriptor(0), 
    m_threadKill(false),
    m_currentState(Input_IDLE),
    m_Rx_Input(new std::vector<char>(CHAR_DEVICE_SIZE)),
    m_Tx_Input(new std::vector<char>(CHAR_DEVICE_SIZE)),
    m_Rx_bytesReceived(0),
    m_Tx_bytesReceived(0),
    m_waitInput(true)
{
    // Initialize m_Rx_Input and m_Tx_Input with zeros
    std::fill(m_Rx_Input->begin(), m_Rx_Input->end(), 0);
    std::fill(m_Tx_Input->begin(), m_Tx_Input->end(), 0);

    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate Input" << std::endl;

}

Input::~Input()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy Input" << std::endl;
    if (m_threadInput.joinable())
    {
        m_threadInput.join();
    }

    m_waitInput = false;

    delete m_Rx_Input;
    delete m_Tx_Input;
}

int Input::openDEV()
{
    m_file_descriptor = open("/dev/KernelInput", O_RDWR);

    if(m_file_descriptor < 0)
    {
        std::cout << "[ERNO] [ I ] Failed to open Device" << std::endl;
        m_threadKill = true;
        return ERROR;
    } 
    else 
    {
        std::cout << "[INFO] [ I ] Device opened successfuly" << std::endl;
        initThread();
    }

    return OK;
}

int Input::dataRX()
{
    /**
     * One way communication Here
     *
     * Rx is not Active in fops
     * in the Input module
     */
    return OK;
}

int Input::dataTX()
{
    int ret = -1;

    /* Wait for data from TCP client */
    std::cout << "[INFO] [ I ] Wait untile waitInput flag is released" << std::endl;
    while(true == m_waitInput)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    m_waitInput = true;

    /* TODO :: Must align received data with what is processed in Kernel */
    ret = write(m_file_descriptor, m_Tx_Input->data(), m_Rx_bytesReceived);

    if (ret == -1)
    {
        std::cout << "[ERNO] [ I ] Cannot write to kernel space" << std::endl;
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
    (*m_Tx_Input)[0] = 0x12; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    (*m_Tx_Input)[1] = 0x34; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    ret = write(m_file_descriptor, m_Tx_Input->data(), 2);

    
    m_Tx_Input->clear(); /* Clear charDevice Rx buffer */

    return OK;
}

int Input::closeDEV()
{
    if (m_file_descriptor >= 0) 
    {
        close(m_file_descriptor);
        m_file_descriptor = -1; // Mark as closed
    }

    m_threadKill = true;
    
    return OK;
}

void Input::initThread()
{
    std::cout << "[INFO] [ I ] Initialize threadInput" << std::endl;
    m_threadInput = std::thread(&Input::threadInput, this);
}

bool Input::isThreadKilled()
{
	return m_threadKill;
}

void Input::threadInput()
{
    while(!m_threadKill) 
    {
        switch(m_currentState)
        {
            case Input_IDLE:
                break;

            case Input_TX:
                std::cout << "[INFO] [ I ] Input_TX mode" << std::endl;

                if(OK != dataTX())
                {
                    std::cout << "[ERNO] [ I ] Cannot write into the Kernel Space" << std::endl;
                }
                else
                {
                    std::cout << "[INFO] [ I ] Data successfuly written into the Kernel Space" << std::endl;
                }

                setInputState(Input_IDLE);
                break;

            case Input_KILL:
                std::cout << "[INFO] [ I ] set Input_KILL mode" << std::endl;
                (*m_Tx_Input)[0] = 0xDE;
                (*m_Tx_Input)[1] = 0xAD;
                write(m_file_descriptor, m_Tx_Input->data(), 2);
                m_Tx_Input->clear();
                setInputState(Input_IDLE);
                m_threadKill = true;
                break;

            default:
                std::cout << "[INFO] [ I ] Unknown mode" << std::endl;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [ I ] Terminate threadInput" << std::endl;
}

void Input::setTx_Input(std::vector<char>* DataRx, int byteReceived)
{
    m_Tx_Input = DataRx;
    m_Rx_bytesReceived = byteReceived;
    std::cout << "[INFO] [ I ] Release waitInput flag" << std::endl;
    m_waitInput = false;
}

void Input::setInputState(Input_stateType newState)
{
    m_currentState = newState;
}
