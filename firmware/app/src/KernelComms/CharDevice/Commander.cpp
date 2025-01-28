/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#include <chrono> // delay
#include <thread> // delay
#include <iostream>
#include <cstring>
#include <iomanip> // Include the <iomanip> header for setw and setfill
#include <fcntl.h> // For open, O_RDWR, etc.
#include <unistd.h>// For close, read, write, etc.

#include "Commander.h"
#include "Types.h"

Commander::Commander() :
m_file_descriptor(-1),
m_threadKill(false),
m_commanderState(COMMANDER_IDLE),
m_instance(this),
m_Rx_Commander(new std::vector<char>(CHAR_DEVICE_SIZE)),
m_Tx_Commander(new std::vector<char>(CHAR_DEVICE_SIZE))
{
    std::cout << "[INFO] [CONSTRUCTOR] " << m_instance << " :: Instantiate Commander" << std::endl;
}

Commander::~Commander()
{
    std::cout << "[INFO] [DESTRUCTOR] " << m_instance << " :: Destroy Commander" << std::endl;

    closeDEV();

    delete m_Rx_Commander;
    delete m_Tx_Commander;
}

void Commander::initBuffers()
{
    std::cout << "[INFO] [COM] Initialise Commander Buffers" << std::endl;
    std::fill(m_Rx_Commander->begin(), m_Rx_Commander->end(), 0);
    std::fill(m_Tx_Commander->begin(), m_Tx_Commander->end(), 0);
}

int Commander::openDEV()
{
    m_file_descriptor = open("/dev/KernelCommander", O_RDWR);

    if(m_file_descriptor < 0)
    {
        std::cout << "[ERNO] [CMD] Failed to open Device" << std::endl;
        return ERROR;
    } 
    else 
    {
        std::cout << "[INFO] [CMD] Device opened successfuly" << std::endl;
        initThread();
    }

    return OK;
}

int Commander::dataRX()
{
    /**
     * One way communication Here
     *
     * Rx is not Active in fops
     * in the Commander module
     */
    return OK;
}

/**
 *
 * TODO :: Need parametrization
 *
 * Only one command at the call
 * of the dataTx function
 *
 */
int Commander::dataTX()
{
    int ret = -1;

    std::cout << "[INFO] [CMD] Command Received :: Sending to Kernel" << std::endl;

    (*m_Tx_Commander)[0] = 0x10;
    (*m_Tx_Commander)[1] = 0xAD;
    ret = write(m_file_descriptor, m_Tx_Commander->data(), 2);

    if (ret == -1)
    {
        std::cout << "[ERNO] [CMD] Cannot write command to kernel space" << std::endl;
        return ERROR;
    }

    for (size_t i = 0; i < CHAR_DEVICE_SIZE; i++)
    {
        (*m_Tx_Commander)[i] = 0x00;
    }

    return OK;
}

int Commander::closeDEV()
{
    if (m_file_descriptor >= 0) 
    {
        close(m_file_descriptor);
        m_file_descriptor = -1; // Mark as closed
    }

    return OK;
}

void Commander::initThread()
{
    std::cout << "[INFO] [CMD] Initialize threadCommander" << std::endl;
    m_threadCommander = std::thread(&Commander::threadCommander, this);
}

void Commander::shutdownThread()
{
    if(false == m_threadKill)
    {
        m_threadKill = true;
    }

    if (m_threadCommander.joinable())
    {
        m_threadCommander.join();
    }
}

bool Commander::isThreadKilled()
{
    return m_threadKill;
}

void Commander::threadCommander()
{
    while (!m_threadKill)
    {
        switch(m_commanderState)
        {
            case COMMANDER_IDLE:
                break;

            case COMMANDER_READ:
                m_commanderState = COMMANDER_IDLE;
                break;

            case COMMANDER_WRITE:
                m_commanderState = COMMANDER_READ;
                break;

            case COMMANDER_DEAD:
                m_commanderState = COMMANDER_READ;
                break;

            case COMMANDER_LOAD:
                m_commanderState = COMMANDER_READ;
                break;

            case COMMANDER_CLEAR:
                m_commanderState = COMMANDER_READ;
                break;

            default:
                std::cout << "[INFO] [CMD] Unknown Command" << std::endl;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [WDG] Terminate threadCommander" << std::endl;
}

Commander* Commander::getInstance()
{
    return m_instance;
}

void Commander::test()
{
    std::cout << "[INFO] [CMD] Test Command Received :: Sending to Kernel" << std::endl;
}
