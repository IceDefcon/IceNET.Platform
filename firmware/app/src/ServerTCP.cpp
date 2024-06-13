/*!
 * 
 * Author: Ice.Marek
 * iceNET Technology 2024
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
    m_file_descriptor(0), 
    m_killThread(false),
    m_portNumber(2555),
    m_serverSocket(-1),
    m_clientSocket(-1),
    m_bytesReceived(0),
    m_clientConnected(false),
    m_ServerTCPRx(new std::vector<char>(TCP_BUFFER_SIZE)),
    m_ServerTCPTx(new std::vector<char>(TCP_BUFFER_SIZE))
{
    std::cout << "[INFO] [CONSTRUCTOR] Instantiate ServerTCP" << std::endl;

    memset(&m_serverAddress, 0, sizeof(m_serverAddress));
    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_addr.s_addr = INADDR_ANY;
    m_serverAddress.sin_port = htons(m_portNumber);

    /* Initialize m_ServerTCPRx and m_ServerTCPTx with zeros */
    std::fill(m_ServerTCPRx->begin(), m_ServerTCPRx->end(), 0);
    std::fill(m_ServerTCPTx->begin(), m_ServerTCPTx->end(), 0);
}

ServerTCP::~ServerTCP() 
{
    std::cout << "[INFO] [DESTRUCTOR] Destroy ServerTCP" << std::endl;

    closeDEV();

    if (m_serverSocket >= 0) 
    {
        close(m_serverSocket);
    }

    if (m_ServerTCPThread.joinable()) 
    {
        m_ServerTCPThread.join();
    }

    delete m_ServerTCPRx;
    delete m_ServerTCPTx;
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
    m_killThread = true;

    return OK;
}

void ServerTCP::initThread()
{
    std::cout << "[INFO] [THREAD] Initialize TCP Server" << std::endl;
    m_ServerTCPThread = std::thread(&ServerTCP::ServerTCPThread, this);
}

bool ServerTCP::isThreadKilled()
{
    return m_killThread;
}

void ServerTCP::ServerTCPThread()
{
    initServer();

    socklen_t clientLength = sizeof(m_clientAddress);

    while (!m_killThread)
    {
        std::cout << "[INFO] [TCP] ServerTCPThread ready for next TCP packet" << std::endl;
        if (false == m_clientConnected)
        {
            /* Wait for the TCP client connection */
            m_clientSocket = accept(m_serverSocket, (struct sockaddr *)&m_clientAddress, &clientLength);

            if(tcpRX() > 0)
            {
                /* TODO :: Set the flag to indicate data ready */
            }
            else
            {
                std::cout << "[INFO] [TCP] Nothing Received :: Cannot set TCP_TO_CHAR in State Machine" << std::endl;
            }

            if (tcpTX() < 0)
            {
                std::cout << "[ERNO] [TCP] Failed to send message" << std::endl;
            }
            else
            {
                std::cout << "[INFO] [TCP] Transfer complete" << std::endl;
            }

            tcpClose();

            /* TODO :: Temporary */
            m_killThread = true;
        }
        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [THREAD] Terminate TCP Server" << std::endl;
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
    int option = 1;
    int ret = -1;

    ret = setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (ret < 0) 
    {
        std::cout << "[ERNO] [TCP] Error setting socket options" << std::endl;
    }
    else
    {
        std::cout << "[INFO] [TCP] Socket option set correctly" << std::endl;
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
#if 0 /* Debug Message */
        (*m_ServerTCPTx)[0] = 0x69; /* i */
        (*m_ServerTCPTx)[1] = 0x63; /* c */
        (*m_ServerTCPTx)[2] = 0x65; /* e */
        (*m_ServerTCPTx)[3] = 0x4E; /* N */
        (*m_ServerTCPTx)[4] = 0x45; /* E */
        (*m_ServerTCPTx)[5] = 0x54; /* T */
        (*m_ServerTCPTx)[6] = '\n'; /* Next line for the GUI console */
#else
        while(false == m_NetworkTrafficIstance->getFeedbackFlag())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        m_ServerTCPTx = m_NetworkTrafficIstance->getNetworkTrafficTx();
        m_NetworkTrafficIstance->resetFeedbackFlag();
#endif

        ret = write(m_clientSocket, m_ServerTCPTx->data(), m_ServerTCPTx->size());
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
        std::cout << "[INFO] [TCP] Client connected to server" << std::endl;
        m_clientConnected = true;

        m_bytesReceived = recv(m_clientSocket, m_ServerTCPRx->data(), TCP_BUFFER_SIZE, 0);

        /**
         * 
         * Here we need a signaling for application state machine
         * to get data obtained from the TCP client
         * 
         */
        if (m_bytesReceived > 0)
        {
            std::cout << "[INFO] [TCP] Received " << m_bytesReceived << " Bytes of data: ";
            for (int i = 0; i < m_bytesReceived; ++i)
            {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_ServerTCPRx)[i]) << " ";
            }
            std::cout << std::endl;
        }
        else if (m_bytesReceived == 0)
        {
            std::cout << "[INFO] [TCP] Connection closed by client" << std::endl;
        }
        else
        {
            std::cout << "[ERNO] [TCP] Error receiving data" << std::endl;
        }

        m_NetworkTrafficIstance->setNetworkTrafficRx(m_ServerTCPRx);
        m_NetworkTrafficIstance->setNetworkTraffic(Kernel_IN_TRANSFER);
    }
    
    /* Resize to actual bytes read */
    // m_ServerTCPRx->resize(m_bytesRead);

    return m_bytesReceived;
}

int ServerTCP::tcpClose() 
{
    if (m_clientSocket >= 0) 
    {
        close(m_clientSocket);
        m_clientSocket = -1;
        m_clientConnected = false;
    }

    return 0;
}

void ServerTCP::setNetworkTrafficIstance(NetworkTraffic* instance)
{
    m_NetworkTrafficIstance = instance;
}
