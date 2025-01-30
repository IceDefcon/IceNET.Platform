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
    m_ioState(IO_IDLE),
    m_ioStatePrev(IO_IDLE),
    m_Rx_CommanderVector(std::make_shared<std::vector<char>>(IO_TRAMSFER_SIZE)),
    m_Tx_CommanderVector(std::make_shared<std::vector<char>>(IO_TRAMSFER_SIZE)),
    m_Rx_Commander(new std::vector<char>(IO_TRAMSFER_SIZE)),
    m_Tx_Commander(new std::vector<char>(IO_TRAMSFER_SIZE)),
    m_Rx_bytesReceived(0),
    m_Tx_bytesReceived(0),
    m_transferComplete(false)
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate Commander" << std::endl;
}

Commander::~Commander()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy Commander" << std::endl;

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

    for (size_t i = 0; i < IO_TRAMSFER_SIZE; i++)
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
    int ret = 0;
    while (!m_threadKill)
    {
        if(m_ioState != m_ioStatePrev)
        {
            std::cout << "[INFO] [CMD] State Commander " << m_ioStatePrev << "->" << m_ioState << " " << getIoStateString(m_ioState) << std::endl;
            m_ioStatePrev = m_ioState;

            switch(m_ioState)
            {
                case IO_IDLE:
                    m_transferComplete = false;
                    break;

                case IO_READ:
                    if(false == m_transferComplete)
                    {
                        std::cout << "[INFO] [CMD] Trying to read from Kernel" << std::endl;
                        ret = read(m_file_descriptor, m_Rx_Commander->data(), IO_TRAMSFER_SIZE);

                        std::cout << "[INFO] [CMD] Received " << ret << " Bytes of data: ";
                        for (int i = 0; i < ret; ++i)
                        {
                            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Rx_Commander)[i]) << " ";
                        }
                        std::cout << std::endl;
                        m_transferComplete = true;
                    }


                    // m_ioState = IO_IDLE;
                    break;

                case IO_WRITE:
                    ret = -1;

                    std::cout << "[INFO] [CMD] Data Received :: Sending to Kernel" << std::endl;

                    ret = write(m_file_descriptor, m_Tx_Commander->data(), m_Tx_bytesReceived);

                    if (ret == -1)
                    {
                        std::cout << "[ERNO] [CMD] Cannot write command to kernel space" << std::endl;
                    }

                    for (size_t i = 0; i < IO_TRAMSFER_SIZE; i++)
                    {
                        (*m_Tx_Commander)[i] = 0x00;
                    }
                    // std::cout << "[INFO] [CMD] Write Command" << std::endl;
                    m_ioState = IO_READ;
                    break;

                case IO_LOAD:
                    // std::cout << "[INFO] [CMD] Load Command" << std::endl;
                    // m_ioState = IO_READ;
                    break;

                case IO_CLEAR:
                    // std::cout << "[INFO] [CMD] Clear Command" << std::endl;
                    // m_ioState = IO_READ;
                    break;

                default:
                    std::cout << "[INFO] [CMD] Unknown Command" << std::endl;
            }
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [WDG] Terminate threadCommander" << std::endl;
}

/**
 * TODO
 *
 * This must be mutex protected
 * to avoid read/write in the
 * same time
 *
 */
void Commander::setIO_State(ioStateType state)
{
    m_ioState = state;
}

ioStateType Commander::getIO_State()
{
    return m_ioState;
}

/* COPY */ int Commander::getRx_Commander(std::vector<char> &dataRx)
{
    dataRx = *m_Rx_Commander;
    return m_Rx_bytesReceived;
}

/* COPY */ void Commander::setTx_Commander(const std::vector<char> &dataTx, int size)
{
    *m_Tx_Commander = dataTx;
    m_Tx_bytesReceived = size;
}

/* SHARE */ void Commander::setTransferPointer(std::shared_ptr<std::vector<char>> transferPointerRx, std::shared_ptr<std::vector<char>> transferPointerTx)
{
    m_Rx_CommanderVector = transferPointerRx;
    m_Tx_CommanderVector = transferPointerTx;
}
