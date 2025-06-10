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

#include "Watchdog.h"
#include "Types.h"

Watchdog::Watchdog() :
    m_file_descriptor(-1),
    m_threadKill(false),
    m_stopFlag(false),
    m_watchdogDead(false),
    m_Rx_Watchdog(new std::vector<char>(WATCHDOG_TRANSFER_SIZE)),
    m_Tx_Watchdog(new std::vector<char>(WATCHDOG_TRANSFER_SIZE))
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate Watchdog" << std::endl;
}

Watchdog::~Watchdog()
{
    shutdownThread();

    closeDEV();

    delete m_Rx_Watchdog;
    delete m_Tx_Watchdog;

    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy Watchdog" << std::endl;
}

void Watchdog::initBuffers()
{
    std::cout << "[INFO] [WDG] Initialise Watchdog Buffers" << std::endl;
    std::fill(m_Rx_Watchdog->begin(), m_Rx_Watchdog->end(), 0);
    std::fill(m_Tx_Watchdog->begin(), m_Tx_Watchdog->end(), 0);
}

int Watchdog::openDEV()
{
    m_file_descriptor = open("/dev/KernelWatchdog", O_RDWR);

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
    int ret = read(m_file_descriptor, m_Rx_Watchdog->data(), WATCHDOG_TRANSFER_SIZE);

    if ((*m_Rx_Watchdog)[0] == (*m_Rx_Watchdog)[1])
    {
        ret = 0;
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

    return OK;
}

void Watchdog::initThread()
{
    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadWatchdog.joinable())
    {
        std::cout << "[INFO] [WDG] threadWatchdog is already running" << std::endl;
        return;
    }

    std::cout << "[INFO] [WDG] Initialize threadWatchdog" << std::endl;

    m_threadKill = false;
    m_threadWatchdog = std::thread(&Watchdog::threadWatchdog, this);
}

void Watchdog::shutdownThread()
{
    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadKill)
    {
        std::cout << "[INFO] [WDG] threadWatchdog is already marked for shutdown" << std::endl;
        return;
    }

    std::cout << "[INFO] [WDG] Killing threadWatchdog" << std::endl;
    m_threadKill = true;

    if (m_threadWatchdog.joinable())
    {
        m_threadWatchdog.join();
        std::cout << "[INFO] [WDG] threadWatchdog has been shut down" << std::endl;
    }
    std::cout << "[INFO] [WDG] threadWatchdog Shutdown" << std::endl;
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
         * as this is happenig when FPGA reveive new data
         */
        if(dataRX() <= 0)
        {
            std::cout << "[ERNO] [WDG] No FPGA Watchdog Signal" << std::endl;

            /**
             *
             * TODO
             *
             * We cannot directly call shutdownThread() from here
             * because it will be called from main later causing
             * multiple call of the thread join()
             *
             * And :: terminate called after throwing an instance of 'std::system_error'
             *
             * Therefore we can only set the flag if watchdog is dead or not
             * And check this flag in main to call thread termination or not
             * This ensure right termination sequence
             *
             */
            m_watchdogDead = true;
        }
        else
        {
            if(false == m_stopFlag)
            {
                setFpgaConfigReady();
                m_stopFlag = true;
            }
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "[INFO] [WDG] Terminate threadWatchdog" << std::endl;
}

bool Watchdog::isWatchdogDead()
{
    return m_watchdogDead;
}

void Watchdog::setFpgaConfigReady()
{
    m_fpgaConfigReady = true;
}

bool Watchdog::getFpgaConfigReady()
{
    return m_fpgaConfigReady;
}
