/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <chrono> // delay
#include <thread> // delay
#include <iostream>
#include <cstring>
#include <iomanip> // Include the <iomanip> header for setw and setfill
#include <fcntl.h> // For open, O_RDWR, etc.
#include <unistd.h>// For close, read, write, etc.

#include "Watchdog.h"
#include "Types.h"

Watchdog::Watchdog() :
    m_file_descriptor(0), 
    m_threadKill(false),
    m_Rx_Watchdog(new std::vector<char>(CHAR_DEVICE_SIZE)),
    m_Tx_Watchdog(new std::vector<char>(CHAR_DEVICE_SIZE))
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate Watchdog" << std::endl;

    /* Initialize m_Rx_Watchdog and m_Tx_Watchdog with zeros */
    std::fill(m_Rx_Watchdog->begin(), m_Rx_Watchdog->end(), 0);
    std::fill(m_Tx_Watchdog->begin(), m_Tx_Watchdog->end(), 0);
}

Watchdog::~Watchdog()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy Watchdog" << std::endl;

    closeDEV();

    if (m_threadWatchdog.joinable())
    {
        m_threadWatchdog.join();
    }

    delete m_Rx_Watchdog;
    delete m_Tx_Watchdog;
}

int Watchdog::openDEV()
{
    m_file_descriptor = open("/dev/Watchdog", O_RDWR);

    if(m_file_descriptor < 0)
    {
        std::cout << "[ERNO] [WDG] Failed to open Device" << std::endl;
        m_threadKill = true;
        return ERROR;
    } 
    else 
    {
        std::cout << "[INFO] [WDG] Device opened successfuly" << std::endl;
        initThread();
    }

    return OK;
}

int Watchdog::dataRX()
{
    int ret;

    ret = read(m_file_descriptor, m_Rx_Watchdog->data(), CHAR_DEVICE_SIZE);

    if (m_Rx_Watchdog->size() >= 2 && (*m_Rx_Watchdog)[0] == (*m_Rx_Watchdog)[1])
    {
        std::cout << "[ERNO] [WDG] [0] Kill the App :: No FPGA Signal" << std::endl;
        return 0;
    }

    return ret;
}

int Watchdog::dataTX()
{
    /**
     * One way communication Here
     *
     * Tx is not Active in fops
     * in the Watchdog module
     */
    return OK;
}

int Watchdog::closeDEV()
{
    if (m_file_descriptor >= 0) 
    {
        close(m_file_descriptor);
        m_file_descriptor = -1; // Mark as closed
    }

    /* TODO :: Temporarily here */
    m_threadKill = true;

    return OK;
}

void Watchdog::initThread()
{
    std::cout << "[INFO] [WDG] Initialize threadWatchdog" << std::endl;
    m_threadWatchdog = std::thread(&Watchdog::threadWatchdog, this);
}

bool Watchdog::isThreadKilled()
{
    return m_threadKill;
}

void Watchdog::threadWatchdog()
{
    while (!m_threadKill)
    {
        /**
         * There is no need for state machine
         * as it is hard to synchronise kernel space
         * with user space, not able to change state
         * in state machine to initiate read operation
         * as this is happenig when FPGA data is reveived
         */
        if(dataRX() <= 0)
        {
            std::cout << "[ERNO] [WDG] [1] Kill the App :: No FPGA Signal" << std::endl;
            m_threadKill = true;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [WDG] Terminate threadWatchdog" << std::endl;
}
