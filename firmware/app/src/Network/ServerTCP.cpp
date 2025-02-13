/*!
 * 
 * Author: Ice.Marek
 * iceNET Technology 2025
 * 
 */
#include <chrono> // delay
#include <thread> // delay
#include <iostream>
#include <cstring>
#include <iomanip> // Include the <iomanip> header for setw and setfill
#include <fcntl.h> // For open, O_RDWR, etc.
#include <unistd.h>// For close, read, write, etc.

#include "ServerTCP.h"
#include "Types.h"

ServerTCP::ServerTCP() :
    m_threadKill(false),
    m_ioState(IO_IDLE),
    m_ioStatePrev(IO_IDLE),
    m_timeoutConnection(0),
    m_timeoutTransfer(0),
    m_portNumber(2555),
    m_serverSocket(-1),
    m_clientSocket(-1),
    m_clientConnected(false),
    m_Rx_ServerTCPVector(std::make_shared<std::vector<char>>(IO_TRANSFER_SIZE)),
    m_Tx_ServerTCPVector(std::make_shared<std::vector<char>>(IO_TRANSFER_SIZE)),
    m_IO_ServerTCPState(std::make_shared<ioStateType>(IO_IDLE))
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate ServerTCP" << std::endl;

    memset(&m_serverAddress, 0, sizeof(m_serverAddress));
    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_addr.s_addr = INADDR_ANY;
    m_serverAddress.sin_port = htons(m_portNumber);
}

ServerTCP::~ServerTCP()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy ServerTCP" << std::endl;
}

int ServerTCP::configureServer()
{
    m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (m_serverSocket < 0)
    {
        std::cout << "[ERNO] [TCP] Error opening socket" << std::endl;
    }

    /**
     *
     * option = 1
     *
     * Allows you to reuse the socket address and port
     * immediately after the socket is closed,
     * even if it's in the TIME_WAIT state
     *
     */
    int ret = -1;
    int option = 1;
    ret = setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (ret < 0)
    {
        std::cout << "[ERNO] [TCP] setsockopt :: SO_REUSEADDR Failed" << std::endl;
    }
    else
    {
        std::cout << "[INFO] [TCP] setsockopt :: SO_REUSEADDR Set correctly" << std::endl;
    }

    /* Set socket timeout options */
    struct timeval tv;
    tv.tv_sec = 1;  /* 5 seconds timeout */
    tv.tv_usec = 0;
    ret = setsockopt(m_serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    if (ret < 0)
    {
        std::cout << "[ERNO] [TCP] setsockopt :: SO_RCVTIMEO Failed" << std::endl;
    }
    else
    {
        std::cout << "[INFO] [TCP] setsockopt :: SO_RCVTIMEO Set correctly" << std::endl;
    }

    ret = bind(m_serverSocket, (struct sockaddr *)&m_serverAddress, sizeof(m_serverAddress));
    if (ret < 0)
    {
        std::cout << "[ERNO] [TCP] Error on binding the socket" << std::endl;
    }
    else
    {
        std::cout << "[INFO] [TCP] Socket bind successfully" << std::endl;
    }

    /**
     *
     * backlog parameter = 5
     *
     * Maximum number of
     * pending connections
     *
     */
    listen(m_serverSocket, 5);
    if (ret < 0)
    {
        std::cout << "[ERNO] [TCP] Error listening for connections" << std::endl;
    }
    else
    {
        std::cout << "[INFO] [TCP] Listening at the TCP Port..." << std::endl;
    }

    return 0;
}

int ServerTCP::tcpTX()
{
    ssize_t ret = -1;

    if (!m_clientConnected)
    {
        std::cerr << "[ERNO] [TCP] No client connected" << std::endl;
    }
    else
    {
        /**
         *
         * This one here is to keep
         * the things running in the GUI
         * as the transfer is check against
         * 0x0000000000000000 so we add 0xEE
         *
         */

        (*m_Rx_ServerTCPVector)[7] = 0xEE;
        ret = write(m_clientSocket, m_Rx_ServerTCPVector->data(), IO_TRANSFER_SIZE);
    }

    return ret;
}

int ServerTCP::tcpRX()
{
    int ret = 0;

    if (m_clientSocket < 0)
    {
        std::cout << "[ERNO] [TCP] Failed to accept the client connection" << std::endl;
    }
    else
    {
        ret = recv(m_clientSocket, m_Tx_ServerTCPVector->data(), IO_TRANSFER_SIZE, 0);

        /* DEAD :: CODE */
        if((*m_Tx_ServerTCPVector)[0] == 0xDE && (*m_Tx_ServerTCPVector)[1] == 0xAD && (*m_Tx_ServerTCPVector)[2] == 0xC0 && (*m_Tx_ServerTCPVector)[3] == 0xDE)
        {
            std::cout << "[INFO] [TCP] 0xDEAD Received" << std::endl;
            return -5;
        }
        /* LOAD :: CODE */
        else if((*m_Tx_ServerTCPVector)[0] == 0x10 && (*m_Tx_ServerTCPVector)[1] == 0xAD && (*m_Tx_ServerTCPVector)[2] == 0xC0 && (*m_Tx_ServerTCPVector)[3] == 0xDE)
        {
            std::cout << "[INFO] [TCP] Send configuration to RAM" << std::endl;
            return -4;
        }
        /* CLEAR :: CODE */
        else if((*m_Tx_ServerTCPVector)[0] == 0xC1 && (*m_Tx_ServerTCPVector)[1] == 0xEA && (*m_Tx_ServerTCPVector)[2] == 0xC0 && (*m_Tx_ServerTCPVector)[3] == 0xDE)
        {
            std::cout << "[INFO] [TCP] Clear DMA from RAM" << std::endl;
            return -3;
        }
        else
        {
            if (ret > 0)
            {
                std::cout << "[INFO] [TCP] Received " << ret << " Bytes of data: ";
                for (int i = 0; i < ret; ++i)
                {
                    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Tx_ServerTCPVector)[i]) << " ";
                }
                std::cout << std::endl;
            }
            else if (ret == 0)
            {
                std::cout << "[INFO] [TCP] Connection closed by client" << std::endl;
            }
            else if(10 == m_timeoutTransfer)
            {
                std::cout << "\r[INFO] [TCP] TCP Transfer timeout :: Shutdown TCP connection" << std::endl;
                m_timeoutTransfer = 0;
                m_clientConnected = false;
                *m_IO_ServerTCPState = IO_IDLE;
            }
            else
            {
#if 0
                if(0 < m_timeoutTransfer) /* TODO :: Quick workaround to avoid log entries concatenation */
                {
                    std::cout << "\r[INFO] [TCP] Nothing received, listening... [" << m_timeoutTransfer << "]" << std::flush;
                }
#endif
                m_timeoutTransfer++;
            }
        }
    }

    return ret;
}

int ServerTCP::tcpClose()
{
    if (m_clientSocket >= 0)
    {
        close(m_clientSocket);
        m_clientSocket = -1;
        m_clientConnected = false;
        *m_IO_ServerTCPState = IO_IDLE;
        m_timeoutTransfer = 0;
    }

    return 0;
}

void ServerTCP::initThread()
{
    std::cout << "[INFO] [TCP] Initialize threadServerTCP" << std::endl;
    m_threadServerTCP = std::thread(&ServerTCP::threadServerTCP, this);
}

void ServerTCP::shutdownThread()
{
    /* Unblock the accept function */
    if (m_serverSocket >= 0)
    {
        close(m_serverSocket);
        m_serverSocket = -1; // Prevent double-closing
    }

    /* Set the threadKill flag */
    if (!m_threadKill)
    {
        m_threadKill = true;
    }

    /* Join the thread if it's joinable */
    if (m_threadServerTCP.joinable())
    {
        m_threadServerTCP.join();
    }
}

bool ServerTCP::isThreadKilled()
{
    return m_threadKill;
}

bool ServerTCP::isClientConnected()
{
    bool ret = false;
    socklen_t clientLength = sizeof(m_clientAddress);

    if(1 < m_timeoutConnection) /* TODO :: Quick workaround to avoid log entries concatenation */
    {
        std::cout << "\r[INFO] [TCP] threadServerTCP waiting for the TCP Client... [" << m_timeoutConnection << "]" << std::flush;
    }

    /* Wait for the TCP client connection */
    m_clientSocket = accept(m_serverSocket, (struct sockaddr *)&m_clientAddress, &clientLength);

    if (m_clientSocket < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            m_timeoutConnection++;
        }
        else
        {
            std::cerr << "[ERROR] [TCP] accept failed: " << strerror(errno) << std::endl;
        }
    }
    else
    {
        std::cout << std::endl;
        std::cout << "[INFO] [TCP] threadServerTCP client connection established" << std::endl;
        m_timeoutConnection = 0;
        ret = true;
    }

    return ret;
}

void ServerTCP::threadServerTCP()
{
    int ret = 0;
    configureServer();

    while (!m_threadKill)
    {
        if (false == m_clientConnected)
        {
            m_clientConnected = isClientConnected();
        }
        else
        {
            m_ioState = (ioStateType)(*m_IO_ServerTCPState);

            if(m_ioState != m_ioStatePrev)
            {
                /**
                 *
                 * TODO :: GLITCH !!
                 *
                 * There is a litle glitch
                 * When client is disconnected
                 * We cannot print state update due
                 * to the else which is not executed
                 *
                 */
                std::cout << "[INFO] [TCP] State ServerTCP " << m_ioStatePrev << "->" << m_ioState << " " << getIoStateString(m_ioState) << std::endl;
                m_ioStatePrev = m_ioState;
            }

            /**
             *
             * INFO
             *
             * Switch Pointer Shared with Commander and DroneCtrl
             *
             * Both state machines change
             * states simultaneously
             * due to share_ptr
             *
             * (1) Commander
             * (2) ServerTCP
             *
             **/
            switch(*m_IO_ServerTCPState)
            {
                case IO_IDLE:
                    if(true == m_clientConnected)
                    {
                        *m_IO_ServerTCPState = IO_TCP_READ;
                        std::cout << "[INFO] [TCP] Read from TCP Client" << std::endl;
                    }
                    break;

                case IO_TCP_READ:
                    ret = tcpRX();

                    if(ret == 0)
                    {
                        std::cout << "[INFO] [TCP] Client disconnected from server" << std::endl;
                        m_clientConnected = false;
                        *m_IO_ServerTCPState = IO_IDLE;
                        m_timeoutTransfer = 0;
                    }
                    else if(ret > 0)
                    {
                        m_timeoutTransfer = 0;
                        *m_IO_ServerTCPState = IO_COM_WRITE;
                    }
                    else if(ret == -5)
                    {
                        tcpClose();
                        std::cout << "[INFO] [TCP] Ready to Kill threadServerTCP" << std::endl;
                        m_threadKill = true;
                    }
                    else if(ret == -4)
                    {
                        std::cout << "[INFO] [TCP] Transfer Data to RAM" << std::endl;
                        //
                        // TODO :: If Needed
                        //
                        // m_instanceRamDisk->dataTX();
                        m_timeoutTransfer = 0;
                    }
                    else if(ret == -3)
                    {
                        std::cout << "[INFO] [TCP] Clear DMA Engine from RAM" << std::endl;
                        //
                        // TODO :: If Needed
                        //
                        // m_instanceRamDisk->clearDma();
                        m_timeoutTransfer = 0;
                    }
                    else
                    {
                        /* TODO :: Client connected but nothing receiver */
                    }
                    break;

                case IO_COM_WRITE: /* Commander is Now Processing Data */
                    /**
                     *
                     * !!! DO NOTHING HERE !!!
                     *
                     */
                    break;

                case IO_COM_READ: /* Commander is Now Processing Data */
                    /**
                     *
                     * !!! DO NOTHING HERE !!!
                     *
                     */
                    break;

                case IO_TCP_WRITE:
                    std::cout << "[INFO] [TCP] Write to TCP Client" << std::endl;

                    if (tcpTX() < 0)
                    {
                        std::cout << "[ERNO] [TCP] Failed to send message" << std::endl;
                    }
                    else
                    {
                        std::cout << "[INFO] [TCP] Transfer complete" << std::endl;
                        m_timeoutTransfer = 0;
                        for (size_t i = 0; i < IO_TRANSFER_SIZE; i++)
                        {
                            (*m_Rx_ServerTCPVector)[i] = 0x00;
                        }
                        *m_IO_ServerTCPState = IO_IDLE; /* Test */
                    }

                    break;

                case IO_LOAD:
                    // std::cout << "[INFO] [TCP] Load Command" << std::endl;
                    break;

                case IO_CLEAR:
                    // std::cout << "[INFO] [TCP] Clear Command" << std::endl;
                    break;

                default:
                    std::cout << "[INFO] [TCP] Unknown Command" << std::endl;
            }
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [TCP] Terminate threadServerTCP" << std::endl;
}

/* SHARE */ void ServerTCP::setTransferPointers(
std::shared_ptr<std::vector<char>> transferPointerRx,
std::shared_ptr<std::vector<char>> transferPointerTx,
std::shared_ptr<ioStateType> transferState)
{
    m_Rx_ServerTCPVector = transferPointerRx;
    m_Tx_ServerTCPVector = transferPointerTx;
    m_IO_ServerTCPState = transferState;
}
