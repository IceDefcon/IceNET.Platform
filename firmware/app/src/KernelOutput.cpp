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

#include "KernelOutput.h"
#include "Types.h"

KernelOutput::KernelOutput() :
    m_file_descriptor(0), 
    m_threadKill(false),
    m_Rx_KernelOutput(new std::vector<char>(CHAR_DEVICE_SIZE)),
    m_Tx_KernelOutput(new std::vector<char>(CHAR_DEVICE_SIZE))
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate KernelOutput" << std::endl;

    /* Initialize m_Rx_KernelOutput and m_Tx_KernelOutput with zeros */
    std::fill(m_Rx_KernelOutput->begin(), m_Rx_KernelOutput->end(), 0);
    std::fill(m_Tx_KernelOutput->begin(), m_Tx_KernelOutput->end(), 0);
}

KernelOutput::~KernelOutput() 
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy KernelOutput" << std::endl;

    closeDEV();

    if (m_threadKernelOutput.joinable()) 
    {
        m_threadKernelOutput.join();
    }

    m_instanceNetworkTraffic = nullptr;

    delete m_Rx_KernelOutput;
    delete m_Tx_KernelOutput;
}

int KernelOutput::openDEV() 
{
    m_file_descriptor = open("/dev/KernelOutput", O_RDWR);

    if(m_file_descriptor < 0)
    {
        std::cout << "[ERNO] [OUT] Failed to open Device" << std::endl;
        m_threadKill = true;
        return ERROR;
    } 
    else 
    {
        std::cout << "[INFO] [OUT] Device opened successfuly" << std::endl;
        initThread();
    }

    return OK;
}

int KernelOutput::dataRX()
{
    int ret;

    std::cout << "[INFO] [OUT] Trying to read from Kernel" << std::endl;
    ret = read(m_file_descriptor, m_Rx_KernelOutput->data(), CHAR_DEVICE_SIZE);

    std::cout << "[INFO] [OUT] Received " << ret << " Bytes of data: ";
    for (int i = 0; i < ret; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Rx_KernelOutput)[i]) << " ";
    }
    std::cout << std::endl;

    return OK;
}

int KernelOutput::dataTX()
{
    return OK;
}

int KernelOutput::closeDEV() 
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

void KernelOutput::initThread()
{
    std::cout << "[INFO] [THREAD] Initialize KernelOutput" << std::endl;
    m_threadKernelOutput = std::thread(&KernelOutput::threadKernelOutput, this);
}

bool KernelOutput::isThreadKilled()
{
    return m_threadKill;
}

void KernelOutput::threadKernelOutput()
{
    while (!m_threadKill)
    {
        std::cout << "[INFO] [OUT] Waiting for next Feedback message" << std::endl;
        
        if(OK != dataTX())
        {
            std::cout << "[ERNO] [OUT] Cannot write into the console" << std::endl;
        }
        else
        {
            if(OK != dataRX())
            {
                std::cout << "[ERNO] [OUT] Cannot read from the console" << std::endl;
            }
            else
            {
                m_instanceNetworkTraffic->setNetworkTrafficTx(m_Rx_KernelOutput);
                m_instanceNetworkTraffic->setNetworkTrafficState(NetworkTraffic_KernelOutput);
                /**
                 * 
                 * TODO
                 * 
                 * Temporarily here to avoid 
                 * system freeze at TCP
                 * thread
                 * 
                 */
                m_threadKill = true;
                
            }
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [THREAD] Terminate KernelOutput" << std::endl;
}

void KernelOutput::setInstance_NetworkTraffic(const std::shared_ptr<NetworkTraffic> instance)
{
    m_instanceNetworkTraffic = instance;
}
