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

#include "Kernel_OUT.h"
#include "Types.h"

Kernel_OUT::Kernel_OUT() :
    m_file_descriptor(0), 
    m_threadKill(false),
    m_Kernel_OUTRx(new std::vector<char>(KERNEL_IN_SIZE)),
    m_Kernel_OUTTx(new std::vector<char>(KERNEL_IN_SIZE)),
    m_instanceNetworkTraffic(std::make_shared<NetworkTraffic>())
{
    std::cout << "[INFO] [CONSTRUCTOR] Instantiate Kernel_OUT" << std::endl;

    /* Initialize m_Kernel_OUTRx and m_Kernel_OUTTx with zeros */
    std::fill(m_Kernel_OUTRx->begin(), m_Kernel_OUTRx->end(), 0);
    std::fill(m_Kernel_OUTTx->begin(), m_Kernel_OUTTx->end(), 0);
}

Kernel_OUT::~Kernel_OUT() 
{
    std::cout << "[INFO] [DESTRUCTOR] Destroy Kernel_OUT" << std::endl;

    closeDEV();

    if (m_Kernel_OUTThread.joinable()) 
    {
        m_Kernel_OUTThread.join();
    }

    m_instanceNetworkTraffic = nullptr;

    delete m_Kernel_OUTRx;
    delete m_Kernel_OUTTx;
}

int Kernel_OUT::openDEV() 
{
    m_file_descriptor = open("/dev/Kernel_OUT", O_RDWR);

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

int Kernel_OUT::dataRX()
{
    int ret;

    ret = read(m_file_descriptor, m_Kernel_OUTRx->data(), KERNEL_IN_SIZE);

    std::cout << "[INFO] [OUT] Received " << ret << " Bytes of data: ";
    for (int i = 0; i < ret; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Kernel_OUTRx)[i]) << " ";
    }
    std::cout << std::endl;

    return OK;
}

int Kernel_OUT::dataTX()
{
    return OK;
}

int Kernel_OUT::closeDEV() 
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

void Kernel_OUT::initThread()
{
    std::cout << "[INFO] [THREAD] Initialize Kernel_OUT" << std::endl;
    m_Kernel_OUTThread = std::thread(&Kernel_OUT::Kernel_OUTThread, this);
}

bool Kernel_OUT::isThreadKilled()
{
    return m_threadKill;
}

void Kernel_OUT::Kernel_OUTThread()
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
                m_instanceNetworkTraffic->setNetworkTrafficTx(m_Kernel_OUTRx);
                m_instanceNetworkTraffic->setNetworkTrafficState(Kernel_OUT_TRANSFER);
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

    std::cout << "[INFO] [THREAD] Terminate Kernel_OUT" << std::endl;
}

void Kernel_OUT::setInstance_NetworkTraffic(std::shared_ptr<NetworkTraffic> instance)
{
    m_instanceNetworkTraffic = instance;
}
