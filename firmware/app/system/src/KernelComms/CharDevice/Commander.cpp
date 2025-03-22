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
m_Rx_CommanderVector(std::make_shared<std::vector<uint8_t>>(IO_TRANSFER_SIZE)),
m_Tx_CommanderVector(std::make_shared<std::vector<uint8_t>>(IO_TRANSFER_SIZE)),
m_IO_CommanderState(std::make_shared<ioStateType>(IO_IDLE)),
m_commandMatrix(CMD_AMOUNT, std::vector<uint8_t>(CMD_LENGTH, 0))  // Initialized with zeros
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate Commander" << std::endl;

    m_commandMatrix[CMD_FPGA_CONFIG] = {0xC0, 0xF1};  /* 0xC0F1(COFI) :: Activate DMA transfer to send IMU's config to FPGA */
    m_commandMatrix[CMD_DMA_RECONFIG] = {0xAE, 0xC0}; /* 0x4ECO(RECO) :: Reconfigure DMA Engine to work with single tramsfer */
    m_commandMatrix[CMD_DMA_CLEAR] = {0xC1, 0xEA}; /* 0xC1EA(CLEA) :: Clear DMA variables used for verification of IMU's config */
}

Commander::~Commander()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy Commander" << std::endl;

    closeDEV();
}

int Commander::openDEV()
{
    m_file_descriptor = open("/dev/KernelCommander", O_RDWR);

    if(m_file_descriptor < 0)
    {
        std::cout << "[ERNO] [CMD] Failed to open Device" << std::endl;
        m_threadKill = true;
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
    /**
     *
     * TODO
     *
     */

    return OK;
}

int Commander::sendCommand(commandType cmd)
{
    int ret = -1;
    std::vector<char>* command = new std::vector<char>(IO_TRANSFER_SIZE);

    std::cout << "[INFO] [CMD] Command Received :: Sending to Kernel" << std::endl;

    (*command)[0] = m_commandMatrix[cmd][0];
    (*command)[1] = m_commandMatrix[cmd][1];

    /* Write command to Kernel Space :: To be processed in Kernel */
    ret = write(m_file_descriptor, command->data(), CMD_LENGTH);

    delete command;

    if (ret == -1)
    {
        std::cout << "[ERNO] [CMD] Cannot write command to kernel space" << std::endl;
        return ERROR;
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
    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadCommander.joinable())
    {
        std::cout << "[INFO] [CMD] threadCommander is already running" << std::endl;
        return;
    }

    std::cout << "[INFO] [CMD] Initialize threadCommander" << std::endl;

    m_threadKill = false;
    m_threadCommander = std::thread(&Commander::threadCommander, this);
}

void Commander::shutdownThread()
{
    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadKill)
    {
        std::cout << "[INFO] [CMD] threadCommander is already marked for shutdown" << std::endl;
        return;
    }

    std::cout << "[INFO] [CMD] Shutdown threadCommander" << std::endl;

    m_threadKill = true;

    if (m_threadCommander.joinable())
    {
        m_threadCommander.join();
        std::cout << "[INFO] [CMD] threadCommander has been shut down" << std::endl;
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
        m_ioState = (ioStateType)(*m_IO_CommanderState);

        if(m_ioState != m_ioStatePrev)
        {
            std::cout << "[INFO] [CMD] State Commander " << m_ioStatePrev << "->" << m_ioState << " " << getIoStateString(m_ioState) << std::endl;
            m_ioStatePrev = m_ioState;
        }

        /**
         *
         * INFO
         *
         * Shared Pointer Switch
         *
         * Both state machines change
         * states simultaneously
         * due to share_ptr
         *
         * [1] Commander
         * [2] DroneCtrl
         *
         **/
        switch(*m_IO_CommanderState)
        {
            case IO_IDLE:
                break;

            case IO_COM_WRITE:
                std::cout << "[INFO] [CMD] Write to Kernel Commander" << std::endl;
                ret = -1;
                std::cout << "[INFO] [CMD] Data Received :: Sending to Kernel" << std::endl;
                printSharedBuffer(m_Tx_CommanderVector);
                ret = write(m_file_descriptor, m_Tx_CommanderVector->data(), IO_TRANSFER_SIZE);

                if (ret == -1)
                {
                    std::cout << "[ERNO] [CMD] Cannot write command to kernel space" << std::endl;
                }
                else
                {
                    for (size_t i = 0; i < IO_TRANSFER_SIZE; i++)
                    {
                        (*m_Tx_CommanderVector)[i] = 0x00;
                    }

                    *m_IO_CommanderState = IO_COM_READ;
                }

                break;

            case IO_COM_READ:
                std::cout << "[INFO] [CMD] Read from Kernel Commander" << std::endl;
                std::cout << "[INFO] [CMD] Trying to read from Kernel" << std::endl;
                /**
                 *
                 * TODO
                 *
                 * This is in blokcing mode
                 * so timeout have to be introduced
                 * here to unlock kernel space
                 *
                 * Or unlock over the timeout inside kernel space
                 *
                 */
                ret = read(m_file_descriptor, m_Rx_CommanderVector->data(), IO_TRANSFER_SIZE);

                if(ret > 0)
                {
                    std::cout << "[INFO] [CMD] Received " << ret << " Bytes of data: ";
                    for (int i = 0; i < ret; ++i)
                    {
                        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Rx_CommanderVector)[i]) << " ";
                    }
                    std::cout << std::endl;
                    *m_IO_CommanderState = IO_IDLE;
                }
                else
                {
                    std::cout << "[ERNO] [CMD] Cannot read from kernel space" << std::endl;
                }

                break;

            default:
                std::cout << "[INFO] [CMD] Unknown Command" << std::endl;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [CMD] Terminate threadCommander" << std::endl;
}

/* SHARE */ void Commander::setTransferPointers(
std::shared_ptr<std::vector<uint8_t>> transferPointerRx,
std::shared_ptr<std::vector<uint8_t>> transferPointerTx,
std::shared_ptr<ioStateType> transferState)
{
    m_Rx_CommanderVector = transferPointerRx;
    m_Tx_CommanderVector = transferPointerTx;
    m_IO_CommanderState = transferState;
}
