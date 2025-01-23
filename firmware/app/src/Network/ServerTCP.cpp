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
    m_timeoutCount(0),
    m_portNumber(2555),
    m_serverSocket(-1),
    m_clientSocket(-1),
    m_clientConnected(false),
    m_Rx_ServerTCP(new std::vector<char>(TCP_SERVER_SIZE)),
    m_Tx_ServerTCP(new std::vector<char>(TCP_SERVER_SIZE)),
    m_Rx_bytesReceived(0),
    m_Tx_bytesReceived(0)
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

    /* Kill the thread */
    closeDEV();

    /* Unblock the accept function */
    if (m_serverSocket >= 0)
    {
        close(m_serverSocket);
    }

    if (m_threadServerTCP.joinable())
    {
        m_threadServerTCP.join();
    }

    m_instanceNetworkTraffic = nullptr;
    m_instanceRamConfig = nullptr;
}

void ServerTCP::InitServerBuffers()
{
    std::cout << "[INFO] [TCP] Initialise ServerTCP Buffers" << std::endl;
    std::fill(m_Rx_ServerTCP->begin(), m_Rx_ServerTCP->end(), 0);
    std::fill(m_Tx_ServerTCP->begin(), m_Tx_ServerTCP->end(), 0);
}

int ServerTCP::openDEV()
{
    initThread();

    return OK;
}

int ServerTCP::dataRX()
{
    return OK;
}

int ServerTCP::dataTX()
{
    return OK;
}

int ServerTCP::closeDEV()
{
    /* TODO :: Temporarily here */
    m_threadKill = true;

    return OK;
}

void ServerTCP::initThread()
{
    std::cout << "[INFO] [TCP] Initialize threadServerTCP" << std::endl;
    m_threadServerTCP = std::thread(&ServerTCP::threadServerTCP, this);
}

bool ServerTCP::isThreadKilled()
{
    return m_threadKill;
}

void ServerTCP::threadServerTCP()
{
    size_t i = 0;

    initServer();

    socklen_t clientLength = sizeof(m_clientAddress);

    while (!m_threadKill)
    {
        /**
         *
         * TODO
         *
         * State Machine would be
         * more suitable for
         * this process
         *
         */
        if (false == m_clientConnected)
        {
            std::cout << "\r[INFO] [TCP] threadServerTCP waiting for the TCP Client... [" << m_timeoutCount << "]" << std::flush;
            /* Wait for the TCP client connection */
            m_clientSocket = accept(m_serverSocket, (struct sockaddr *)&m_clientAddress, &clientLength);

            if (m_clientSocket < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    m_timeoutCount++;
                    continue; /* Continue waiting for the client */
                }
                else
                {
                    std::cout << std::endl;
                    std::cerr << "[ERROR] [TCP] accept failed: " << strerror(errno) << std::endl;
                }
            }
            else
            {
                std::cout << std::endl;
                std::cout << "[INFO] [TCP] threadServerTCP client connection established" << std::endl;
                m_clientConnected = true;
                m_timeoutCount = 0;
            }
        }
        else
        {
            int ret = tcpRX();

            if(ret > 0)
            {
                std::cout << "[INFO] [TCP] Sending data to NetworkTraffic" << std::endl;
                m_instanceNetworkTraffic->setNetworkTrafficRx(m_Rx_ServerTCP, m_Rx_bytesReceived);
                std::cout << "[INFO] [TCP] Set NetworkTraffic_Input mode" << std::endl;
                m_instanceNetworkTraffic->setNetworkTrafficState(NetworkTraffic_Input);

                if (tcpTX() < 0)
                {
                    std::cout << "[ERNO] [TCP] Failed to send message" << std::endl;
                }
                else
                {
                    std::cout << "[INFO] [TCP] Transfer complete" << std::endl;
                    m_timeoutCount = 0;
                }
            }
            else if(ret == -5)
            {
                tcpClose();
                std::cout << "[INFO] [TCP] Ready to Kill threadServerTCP" << std::endl;
                m_instanceNetworkTraffic->setNetworkTrafficState(NetworkTraffic_KILL);
                m_threadKill = true;
            }
            else if(ret == -4)
            {
                std::cout << "[INFO] [TCP] Transfer Data to RAM" << std::endl;
                m_instanceRamConfig->dataTX();
                m_timeoutCount = 0;
            }
            else if(ret == -3)
            {
                std::cout << "[INFO] [TCP] Clear DMA Engine from RAM" << std::endl;
                m_instanceRamConfig->clearDma();
                m_timeoutCount = 0;
            }
            else if(ret == 0)
            {
                std::cout << "[INFO] [TCP] Client disconnected from server" << std::endl;
                m_clientConnected = false;
                m_timeoutCount = 0;
            }
            else
            {
                /* TODO :: Client connected but nothing receiver */
            }
        }

        for (i = 0; i < TCP_SERVER_SIZE; i++)
        {
            (*m_Rx_ServerTCP)[i] = 0x00;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [TCP] Terminate threadServerTCP" << std::endl;
}

int ServerTCP::initServer()
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

    /**
     * As in Output we have here
     * lack of synchronisation due to
     * the trigger that is happemning
     * on the client side :: GUI
     *
     * Therefore :: No need for State Machine
     */
    if (!m_clientConnected)
    {
        std::cerr << "[ERNO] [TCP] No client connected" << std::endl;
    }
    else
    {
        while(false == m_instanceNetworkTraffic->getFeedbackFlag())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        m_Tx_ServerTCP = m_instanceNetworkTraffic->getNetworkTrafficTx();
        m_instanceNetworkTraffic->resetFeedbackFlag();
        std::cout << "[INFO] [TCP] Received " << m_Tx_ServerTCP->size() << " Bytes of data: ";
        for (int i = 0; i < (int)m_Tx_ServerTCP->size(); ++i)
        {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Tx_ServerTCP)[i]) << " ";
        }
        std::cout << std::endl;

        /**
         *
         * This one here is to keep
         * the things running in the GUI
         * as the transfer is check against
         * 0x0000000000000000 so we add 0xEE
         *
         */

        (*m_Tx_ServerTCP)[7] = 0xEE;
        ret = write(m_clientSocket, m_Tx_ServerTCP->data(), m_Tx_ServerTCP->size());
        (*m_Tx_ServerTCP)[7] = 0x00; /* Now clear me !! */
    }

    return ret;
}

int ServerTCP::tcpRX()
{
    if (m_clientSocket < 0)
    {
        std::cout << "[ERNO] [TCP] Failed to accept the client connection" << std::endl;
    }
    else
    {
        m_Rx_bytesReceived = recv(m_clientSocket, m_Rx_ServerTCP->data(), TCP_SERVER_SIZE, 0);

        /* DEAD :: CODE */
        if((*m_Rx_ServerTCP)[0] == 0xDE && (*m_Rx_ServerTCP)[1] == 0xAD && (*m_Rx_ServerTCP)[2] == 0xC0 && (*m_Rx_ServerTCP)[3] == 0xDE)
        {
            std::cout << std::endl;
            std::cout << "[INFO] [TCP] 0xDEAD Received" << std::endl;
            return -5;
        }
        /* LOAD :: CODE */
        else if((*m_Rx_ServerTCP)[0] == 0x10 && (*m_Rx_ServerTCP)[1] == 0xAD && (*m_Rx_ServerTCP)[2] == 0xC0 && (*m_Rx_ServerTCP)[3] == 0xDE)
        {
            std::cout << std::endl;
            std::cout << "[INFO] [TCP] Send configuration to RAM" << std::endl;
            return -4;
        }
        /* CLEAR :: CODE */
        else if((*m_Rx_ServerTCP)[0] == 0xC1 && (*m_Rx_ServerTCP)[1] == 0xEA && (*m_Rx_ServerTCP)[2] == 0xC0 && (*m_Rx_ServerTCP)[3] == 0xDE)
        {
            std::cout << std::endl;
            std::cout << "[INFO] [TCP] Clear DMA from RAM" << std::endl;
            return -3;
        }
        else
        {
            if (m_Rx_bytesReceived > 0)
            {
                std::cout << std::endl;
                std::cout << "[INFO] [TCP] Received " << m_Rx_bytesReceived << " Bytes of data: ";
                for (int i = 0; i < m_Rx_bytesReceived; ++i)
                {
                    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Rx_ServerTCP)[i]) << " ";
                }
                std::cout << std::endl;
            }
            else if (m_Rx_bytesReceived == 0)
            {
                std::cout << std::endl;
                std::cout << "[INFO] [TCP] Connection closed by client" << std::endl;
            }
            else
            {
                std::cout << "\r[INFO] [TCP] Nothing received, listening... [" << m_timeoutCount << "]" << std::flush;

                m_timeoutCount++;
            }
        }
    }

    return m_Rx_bytesReceived;
}

int ServerTCP::tcpClose()
{
    if (m_clientSocket >= 0)
    {
        close(m_clientSocket);
        m_clientSocket = -1;
        m_clientConnected = false;
        m_timeoutCount = 0;
    }

    return 0;
}

void ServerTCP::setInstance_NetworkTraffic(const std::shared_ptr<NetworkTraffic> instance)
{
    m_instanceNetworkTraffic = instance;
}

void ServerTCP::setInstance_RamConfig(const std::shared_ptr<RamConfig> instance)
{
    m_instanceRamConfig = instance;
}
