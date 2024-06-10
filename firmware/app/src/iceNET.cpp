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

#include "iceNET.h"
#include "types.h"

iceNET::iceNET() :
    m_file_descriptor(0), 
    m_killThread(false),
    m_iceNETRx(new std::vector<char>(ICE_NET_BUFFER_SIZE)),
    m_iceNETTx(new std::vector<char>(ICE_NET_BUFFER_SIZE))
{
    std::cout << "[INFO] [CONSTRUCTOR] Instantiate iceNET" << std::endl;

    /* Initialize m_iceNETRx and m_iceNETTx with zeros */
    std::fill(m_iceNETRx->begin(), m_iceNETRx->end(), 0);
    std::fill(m_iceNETTx->begin(), m_iceNETTx->end(), 0);
}

iceNET::~iceNET() 
{
    std::cout << "[INFO] [DESTRUCTOR] Destroy iceNET" << std::endl;

    closeDEV();

    if (m_iceNETThread.joinable()) 
    {
        m_iceNETThread.join();
    }
}

int iceNET::openDEV() 
{
    m_file_descriptor = open("/dev/iceNET", O_RDWR);

    if(m_file_descriptor < 0)
    {
        std::cout << "[ERNO] [NET] Failed to open Device" << std::endl;
        m_killThread = true;
        return ERROR;
    } 
    else 
    {
        std::cout << "[INFO] [NET] Device opened successfuly" << std::endl;
        initThread();
    }

    return OK;
}

int iceNET::dataRX()
{

    int ret;

    ret = read(m_file_descriptor, m_iceNETRx->data(), ICE_NET_BUFFER_SIZE);

    std::cout << "[INFO] [NET] Received " << ret << " Bytes of data: ";
    for (int i = 0; i < ret; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_iceNETRx)[i]) << " ";
    }
    std::cout << std::endl;

    return OK;
}

int iceNET::dataTX()
{
    return OK;
}

int iceNET::closeDEV() 
{
    if (m_file_descriptor >= 0) 
    {
        close(m_file_descriptor);
        m_file_descriptor = -1; // Mark as closed
    }

    /* TODO :: Temporarily here */
    m_killThread = true;

    return OK;
}

void iceNET::initThread()
{
    std::cout << "[INFO] [NET] Init the iceNETThread" << std::endl;
    m_iceNETThread = std::thread(&iceNET::iceNETThread, this);
}

bool iceNET::isThreadKilled()
{
    return m_killThread;
}

void iceNET::iceNETThread()
{
    std::cout << "[INFO] [NET] Enter iceNETThread" << std::endl;

    while (!m_killThread)
    {
        std::cout << "[INFO] [NET] Waiting for next Feedback message" << std::endl;
        
        if(OK != dataTX())
        {
            std::cout << "[ERNO] [NET] Cannot write into the console" << std::endl;
        }
        else
        {
            if(OK != dataRX())
            {
                std::cout << "[ERNO] [NET] Cannot read from the console" << std::endl;
            }

            /* TODO :: Set the flag to indicate the data is ready */

        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [NET] Terminate iceNETThread" << std::endl;
}