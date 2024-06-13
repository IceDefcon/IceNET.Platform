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

#include "outputCOM.h"
#include "types.h"

outputCOM::outputCOM() :
    m_file_descriptor(0), 
    m_killThread(false),
    m_outputCOMRx(new std::vector<char>(ICE_NET_BUFFER_SIZE)),
    m_outputCOMTx(new std::vector<char>(ICE_NET_BUFFER_SIZE))
{
    std::cout << "[INFO] [CONSTRUCTOR] Instantiate outputCOM" << std::endl;

    /* Initialize m_outputCOMRx and m_outputCOMTx with zeros */
    std::fill(m_outputCOMRx->begin(), m_outputCOMRx->end(), 0);
    std::fill(m_outputCOMTx->begin(), m_outputCOMTx->end(), 0);
}

outputCOM::~outputCOM() 
{
    std::cout << "[INFO] [DESTRUCTOR] Destroy outputCOM" << std::endl;

    closeDEV();

    if (m_outputCOMThread.joinable()) 
    {
        m_outputCOMThread.join();
    }
}

int outputCOM::openDEV() 
{
    m_file_descriptor = open("/dev/outputCOM", O_RDWR);

    if(m_file_descriptor < 0)
    {
        std::cout << "[ERNO] [outputCOM] Failed to open Device" << std::endl;
        m_killThread = true;
        return ERROR;
    } 
    else 
    {
        std::cout << "[INFO] [outputCOM] Device opened successfuly" << std::endl;
        initThread();
    }

    return OK;
}

int outputCOM::dataRX()
{
    int ret;

    ret = read(m_file_descriptor, m_outputCOMRx->data(), ICE_NET_BUFFER_SIZE);

    std::cout << "[INFO] [outputCOM] Received " << ret << " Bytes of data: ";
    for (int i = 0; i < ret; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_outputCOMRx)[i]) << " ";
    }
    std::cout << std::endl;

    return OK;
}

int outputCOM::dataTX()
{
    return OK;
}

int outputCOM::closeDEV() 
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

void outputCOM::initThread()
{
    std::cout << "[INFO] [THREAD] Initialize outputCOM" << std::endl;
    m_outputCOMThread = std::thread(&outputCOM::outputCOMThread, this);
}

bool outputCOM::isThreadKilled()
{
    return m_killThread;
}

void outputCOM::outputCOMThread()
{
    while (!m_killThread)
    {
        std::cout << "[INFO] [outputCOM] Waiting for next Feedback message" << std::endl;
        
        if(OK != dataTX())
        {
            std::cout << "[ERNO] [outputCOM] Cannot write into the console" << std::endl;
        }
        else
        {
            if(OK != dataRX())
            {
                std::cout << "[ERNO] [outputCOM] Cannot read from the console" << std::endl;
            }
            else
            {
                m_StateMachineIstance->setStateMachineTx(m_outputCOMRx);
                m_StateMachineIstance->setStateMachine(outputCOM_TRANSFER);
                /**
                 * 
                 * TODO
                 * 
                 * Temporarily here to avoid 
                 * system freeze at TCP
                 * thread
                 * 
                 */
                m_killThread = true;
                
            }
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [THREAD] Terminate outputCOM" << std::endl;
}

void outputCOM::setStateMachineIstance(stateMachine* instance)
{
    m_StateMachineIstance = instance;
}
