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

#include "tcpServer.h"
#include "types.h"

tcpServer::tcpServer() :
    m_file_descriptor(0), 
    m_killThread(false),
    m_portNumber(2555),
    m_serverSocket(-1),
    m_clientSocket(-1),
    m_bytesReceived(0),
    m_clientConnected(false),
    m_tcpServerRx(new std::vector<char>(TCP_BUFFER_SIZE)),
    m_tcpServerTx(new std::vector<char>(TCP_BUFFER_SIZE))
{
    std::cout << "[INFO] [CONSTRUCTOR] Instantiate tcpServer" << std::endl;

    memset(&m_serverAddress, 0, sizeof(m_serverAddress));
    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_addr.s_addr = INADDR_ANY;
    m_serverAddress.sin_port = htons(m_portNumber);

    /* Initialize m_tcpServerRx and m_tcpServerTx with zeros */
    std::fill(m_tcpServerRx->begin(), m_tcpServerRx->end(), 0);
    std::fill(m_tcpServerTx->begin(), m_tcpServerTx->end(), 0);
}

tcpServer::~tcpServer() 
{
    std::cout << "[INFO] [DESTRUCTOR] Destroy tcpServer" << std::endl;

    closeDEV();

    if (m_serverSocket >= 0) 
    {
        close(m_serverSocket);
    }

    if (m_tcpServerThread.joinable()) 
    {
        m_tcpServerThread.join();
    }

    delete m_tcpServerRx;
    delete m_tcpServerTx;
}

int tcpServer::openDEV() 
{
    initThread();

    return OK;
}

int tcpServer::dataRX()
{
    return OK;
}

int tcpServer::dataTX()
{
    return OK;
}

int tcpServer::closeDEV() 
{
    /* TODO :: Temporarily here */
    m_killThread = true;

    return OK;
}

void tcpServer::initThread()
{
    std::cout << "[INFO] [NET] Init the tcpServerThread" << std::endl;
    m_tcpServerThread = std::thread(&tcpServer::tcpServerThread, this);
}

bool tcpServer::isThreadKilled()
{
    return m_killThread;
}

void tcpServer::tcpServerThread()
{
    std::cout << "[INFO] [NET] Enter tcpServerThread" << std::endl;

    initServer();

    socklen_t clientLength = sizeof(m_clientAddress);

    while (!m_killThread)
    {
        std::cout << "[INFO] [NET] tcpServerThread ready for next TCP packet" << std::endl;
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
                std::cout << "[INFO] [NET] Nothing Received :: Cannot set TCP_TO_CHAR in State Machine" << std::endl;
            }

            if (tcpTX() < 0)
            {
                std::cout << "[ERNO] [NET] Failed to send message" << std::endl;
            }
            else
            {
                std::cout << "[INFO] [NET] Transfer complete" << std::endl;
            }

            tcpClose();

        }
        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [NET] Terminate tcpServerThread" << std::endl;
}

int tcpServer::initServer() 
{
    m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    if (m_serverSocket < 0)
    {
        std::cout << "[ERNO] [NET] Error opening socket" << std::endl;
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
        std::cout << "[ERNO] [NET] Error setting socket options" << std::endl;
    }
    else
    {
        std::cout << "[INFO] [NET] Socket option set correctly" << std::endl;
    }

    ret = bind(m_serverSocket, (struct sockaddr *)&m_serverAddress, sizeof(m_serverAddress));
    if (ret < 0) 
    {
        std::cout << "[ERNO] [NET] Error on binding the socket" << std::endl;
    }
    else
    {
        std::cout << "[INFO] [NET] Socket bind successfully" << std::endl;
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
        std::cout << "[ERNO] [NET] Error listening for connections" << std::endl;
    }
    else
    {
        std::cout << "[INFO] [NET] Listening at the TCP Port..." << std::endl;
    }

    return 0;
}

int tcpServer::tcpTX()
{
    ssize_t ret = -1;

    if (!m_clientConnected) 
    {
        std::cerr << "[ERNO] [NET] No client connected" << std::endl;
    }
    else
    {
        (*m_tcpServerTx)[0] = 0x69; /* i */
        (*m_tcpServerTx)[1] = 0x63; /* c */
        (*m_tcpServerTx)[2] = 0x65; /* e */
        (*m_tcpServerTx)[3] = 0x4E; /* N */
        (*m_tcpServerTx)[4] = 0x45; /* E */
        (*m_tcpServerTx)[5] = 0x54; /* T */
        (*m_tcpServerTx)[6] = '\n'; /* Next line for the GUI console */

        ret = write(m_clientSocket, m_tcpServerTx->data(), m_tcpServerTx->size());
    }

    return ret;
}

int tcpServer::tcpRX()
{
    if (m_clientSocket < 0)
    {
        std::cout << "[ERNO] [NET] Failed to accept the client connection" << std::endl;
    }
    else
    {
        std::cout << "[INFO] [NET] Client connected to server" << std::endl;
        m_clientConnected = true;

        m_bytesReceived = recv(m_clientSocket, m_tcpServerRx->data(), TCP_BUFFER_SIZE, 0);

        /**
         * 
         * Here we need a signaling for application state machine
         * to get data obtained from the TCP client
         * 
         */
        if (m_bytesReceived > 0)
        {
            std::cout << "[INFO] [NET] Received " << m_bytesReceived << " Bytes of data: ";
            for (int i = 0; i < m_bytesReceived; ++i)
            {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_tcpServerRx)[i]) << " ";
            }
            std::cout << std::endl;
        }
        else if (m_bytesReceived == 0)
        {
            std::cout << "[INFO] [NET] Connection closed by client" << std::endl;
        }
        else
        {
            std::cout << "[ERNO] [NET] Error receiving data" << std::endl;
        }

        m_StateMachineIstance->setStateMachineRx(m_tcpServerRx);
        m_StateMachineIstance->setStateMachine(iceCOM_TRANSFER);
    }
    
    /* Resize to actual bytes read */
    // m_tcpServerRx->resize(m_bytesRead);

    return m_bytesReceived;
}

int tcpServer::tcpClose() 
{
    if (m_clientSocket >= 0) 
    {
        close(m_clientSocket);
        m_clientSocket = -1;
        m_clientConnected = false;
    }

    return 0;
}

void tcpServer::setStateMachineIstance(stateMachine* instance)
{
    m_StateMachineIstance = instance;
}
