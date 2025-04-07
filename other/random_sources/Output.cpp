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

#include "Output.h"
#include "Types.h"

Output::Output() :
    m_file_descriptor(0), 
    m_threadKill(false),
    m_Rx_Output(new std::vector<char>(CHAR_DEVICE_SIZE)),
    m_Tx_Output(new std::vector<char>(CHAR_DEVICE_SIZE))
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate Output" << std::endl;
}

Output::~Output()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy Output" << std::endl;

    closeDEV();

    if (m_threadOutput.joinable())
    {
        m_threadOutput.join();
    }

    m_instanceNetworkTraffic = nullptr;

    delete m_Rx_Output;
    delete m_Tx_Output;
}

void Output::initBuffers()
{
    std::cout << "[INFO] [ O ] Initialise Output Buffers" << std::endl;
    std::fill(m_Rx_Output->begin(), m_Rx_Output->end(), 0);
    std::fill(m_Tx_Output->begin(), m_Tx_Output->end(), 0);
}

int Output::openDEV()
{
    m_file_descriptor = open("/dev/KernelOutput", O_RDWR);

    if(m_file_descriptor < 0)
    {
        std::cout << "[ERNO] [ O ] Failed to open Device" << std::endl;
        m_threadKill = true;
        return ERROR;
    } 
    else 
    {
        std::cout << "[INFO] [ O ] Device opened successfuly" << std::endl;
        initThread();
    }

    return OK;
}

int Output::dataRX()
{
    int ret;

    std::cout << "[INFO] [ O ] Trying to read from Kernel" << std::endl;
    ret = read(m_file_descriptor, m_Rx_Output->data(), CHAR_DEVICE_SIZE);

    std::cout << "[INFO] [ O ] Received " << ret << " Bytes of data: ";
    for (int i = 0; i < ret; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Rx_Output)[i]) << " ";
    }
    std::cout << std::endl;

    if (m_Rx_Output->size() >= 2 && (*m_Rx_Output)[0] == 0xDE && (*m_Rx_Output)[1] == 0xAD)
    {
        std::cout << "[INFO] [ O ] Kill SIGNAL Received" << std::endl;
        return 0;
    }

    return ret;
}

int Output::dataTX()
{
    /**
     * One way communication Here
     *
     * Tx is not Active in fops
     * in the Output module
     */
    return OK;
}

int Output::closeDEV()
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

void Output::initThread()
{
    std::cout << "[INFO] [ O ] Initialize threadOutput" << std::endl;
    m_threadOutput = std::thread(&Output::threadOutput, this);
}

bool Output::isThreadKilled()
{
    return m_threadKill;
}

void Output::threadOutput()
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
        std::cout << "[INFO] [ O ] Waiting for next Feedback message" << std::endl;

        if(dataRX() > 0)
        {
            m_instanceNetworkTraffic->setNetworkTrafficTx(m_Rx_Output);
            m_instanceNetworkTraffic->setNetworkTrafficState(NetworkTraffic_Output);
        }
        else
        {
            std::cout << "[INFO] [ O ] Killing Thread" << std::endl;
            m_threadKill = true;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "[INFO] [ O ] Terminate threadOutput" << std::endl;
}

void Output::setInstance_NetworkTraffic(const std::shared_ptr<NetworkTraffic> instance)
{
    m_instanceNetworkTraffic = instance;
}
