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

#include "serverTCP.h"
#include "types.h"

serverTCP::serverTCP() :
    m_file_descriptor(0), 
    m_killThread(false),
    m_portNumber(2555),
    m_serverSocket(-1),
    m_clientSocket(-1),
    m_bytesReceived(0),
    m_clientConnected(false),
    m_serverTCPRx(new std::vector<char>(TCP_BUFFER_SIZE)),
    m_serverTCPTx(new std::vector<char>(TCP_BUFFER_SIZE))
{
    std::cout << "[INFO] [CONSTRUCTOR] Instantiate serverTCP" << std::endl;

    memset(&m_serverAddress, 0, sizeof(m_serverAddress));
    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_addr.s_addr = INADDR_ANY;
    m_serverAddress.sin_port = htons(m_portNumber);

    /* Initialize m_serverTCPRx and m_serverTCPTx with zeros */
    std::fill(m_serverTCPRx->begin(), m_serverTCPRx->end(), 0);
    std::fill(m_serverTCPTx->begin(), m_serverTCPTx->end(), 0);
}

serverTCP::~serverTCP() 
{
    std::cout << "[INFO] [DESTRUCTOR] Destroy serverTCP" << std::endl;

    closeDEV();

    if (m_serverSocket >= 0) 
    {
        close(m_serverSocket);
    }

    if (m_serverTCPThread.joinable()) 
    {
        m_serverTCPThread.join();
    }

    delete m_serverTCPRx;
    delete m_serverTCPTx;
}

int serverTCP::openDEV() 
{
    initThread();

    return OK;
}

int serverTCP::dataRX()
{
    return OK;
}

int serverTCP::dataTX()
{
    return OK;
}

int serverTCP::closeDEV() 
{
    /* TODO :: Temporarily here */
    m_killThread = true;

    return OK;
}

void serverTCP::initThread()
{
    std::cout << "[INFO] [THREAD] Initialize TCP Server" << std::endl;
    m_serverTCPThread = std::thread(&serverTCP::serverTCPThread, this);
}

bool serverTCP::isThreadKilled()
{
    return m_killThread;
}

void serverTCP::serverTCPThread()
{
    initServer();

    socklen_t clientLength = sizeof(m_clientAddress);

    while (!m_killThread)
    {
        std::cout << "[INFO] [TCP] serverTCPThread ready for next TCP packet" << std::endl;
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

int serverTCP::initServer() 
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

int serverTCP::tcpTX()
{
    ssize_t ret = -1;

    if (!m_clientConnected) 
    {
        std::cerr << "[ERNO] [TCP] No client connected" << std::endl;
    }
    else
    {
#if 0 /* Debug Message */
        (*m_serverTCPTx)[0] = 0x69; /* i */
        (*m_serverTCPTx)[1] = 0x63; /* c */
        (*m_serverTCPTx)[2] = 0x65; /* e */
        (*m_serverTCPTx)[3] = 0x4E; /* N */
        (*m_serverTCPTx)[4] = 0x45; /* E */
        (*m_serverTCPTx)[5] = 0x54; /* T */
        (*m_serverTCPTx)[6] = '\n'; /* Next line for the GUI console */
#else
        while(false == m_StateMachineIstance->getFeedbackFlag())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        m_serverTCPTx = m_StateMachineIstance->getStateMachineTx();
        m_StateMachineIstance->resetFeedbackFlag();
#endif

        ret = write(m_clientSocket, m_serverTCPTx->data(), m_serverTCPTx->size());
    }

    return ret;
}

int serverTCP::tcpRX()
{
    if (m_clientSocket < 0)
    {
        std::cout << "[ERNO] [TCP] Failed to accept the client connection" << std::endl;
    }
    else
    {
        std::cout << "[INFO] [TCP] Client connected to server" << std::endl;
        m_clientConnected = true;

        m_bytesReceived = recv(m_clientSocket, m_serverTCPRx->data(), TCP_BUFFER_SIZE, 0);

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
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_serverTCPRx)[i]) << " ";
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

        m_StateMachineIstance->setStateMachineRx(m_serverTCPRx);
        m_StateMachineIstance->setStateMachine(inputCOM_TRANSFER);
    }
    
    /* Resize to actual bytes read */
    // m_serverTCPRx->resize(m_bytesRead);

    return m_bytesReceived;
}

int serverTCP::tcpClose() 
{
    if (m_clientSocket >= 0) 
    {
        close(m_clientSocket);
        m_clientSocket = -1;
        m_clientConnected = false;
    }

    return 0;
}

void serverTCP::setStateMachineIstance(stateMachine* instance)
{
    m_StateMachineIstance = instance;
}
