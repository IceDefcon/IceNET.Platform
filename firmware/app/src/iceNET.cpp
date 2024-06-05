/*!
 * 
 * Author: Ice.Marek and Ventor
 * IceNET Technology 2024
 * 
 */
#include <chrono> // delay
#include <thread> // delay
#include <iostream>
#include <cstring>
#include <iomanip> // Include the <iomanip> header for setw and setfill
#include <unistd.h>

#include "iceNET.h"

iceNET::iceNET(int portNumber):
m_portNumber(portNumber),
m_serverSocket(-1),
m_clientSocket(-1),
m_clientConnected(false),
m_iceNETThread(),
m_killThread(false),
m_tcpServerRx(TCP_BUFFER_SIZE),
m_tcpServerTx(TCP_BUFFER_SIZE),
m_tcpRxReady(false),
m_tcpTxReady(false),
m_bytesRead(0)
{
    std::cout << "[INFO] [CONSTRUCTOR] Initialise iceNET Object" << std::endl;

    memset(&m_serverAddress, 0, sizeof(m_serverAddress));
    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_addr.s_addr = INADDR_ANY;
    m_serverAddress.sin_port = htons(m_portNumber);

    /* Initialize m_tcpServerRx and m_tcpServerTx with zeros */
    std::fill(m_tcpServerRx.begin(), m_tcpServerRx.end(), 0);
    std::fill(m_tcpServerTx.begin(), m_tcpServerTx.end(), 0);
}

iceNET::~iceNET() 
{
    std::cout << "[INFO] [DESTRUCTOR] Destroy iceNET Object" << std::endl;

    closeCOM();

    if (m_serverSocket >= 0) 
    {
        close(m_serverSocket);
    }

    if (m_iceNETThread.joinable()) 
    {
        m_iceNETThread.join();
    }
}

int iceNET::openCOM() 
{
    m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    if (m_serverSocket < 0)
    {
        std::cout << "[ERROR] [NET] Error opening socket" << std::endl;
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
        std::cout << "[ERROR] [NET] Error setting socket options" << std::endl;
    }
    else
    {
        std::cout << "[INFO] [NET] Socket option set correctly" << std::endl;
    }

    ret = bind(m_serverSocket, (struct sockaddr *)&m_serverAddress, sizeof(m_serverAddress));
    if (ret < 0) 
    {
        std::cout << "[ERROR] [NET] Error on binding the socket" << std::endl;
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
        std::cout << "[ERROR] [NET] Error listening for connections" << std::endl;
    }
    else
    {
        std::cout << "[INFO] [NET] Listening at the TCP Port..." << std::endl;
        initThread();
    }

    return 0;
}

int iceNET::dataTX() 
{
    ssize_t ret = -1;

    if (!m_clientConnected) 
    {
        std::cerr << "Socket Server: no client connected" << std::endl;
    }
    else
    {
        m_tcpServerTx[0] = 0x69; /* i */
        m_tcpServerTx[1] = 0x63; /* c */
        m_tcpServerTx[2] = 0x65; /* e */
        m_tcpServerTx[3] = 0x4E; /* N */
        m_tcpServerTx[4] = 0x45; /* E */
        m_tcpServerTx[5] = 0x54; /* T */
        m_tcpServerTx[6] = '\n'; /* Next line for the GUI console */

        ret = write(m_clientSocket, m_tcpServerTx.data(), m_tcpServerTx.size());
    }

    return ret;
}

int iceNET::dataRX()
{
    if (m_clientSocket < 0)
    {
        std::cout << "[ERROR] [NET] Failed to accept the client connection" << std::endl;
    }
    else
    {
        std::cout << "[INFO] [NET] Client connected to server" << std::endl;
        m_clientConnected = true;

        m_bytesReceived = recv(m_clientSocket, m_tcpServerRx.data(), TCP_BUFFER_SIZE, 0);

        /**
         * 
         * Here we need a signaling for the char Device
         * to get data obtained from the TCP client
         * 
         */

        if (m_bytesReceived > 0)
        {
            std::cout << "[INFO] [NET] Received " << m_bytesReceived << " Bytes of data: ";
            for (int i = 0; i < m_bytesReceived; ++i)
            {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(m_tcpServerRx.data()[i]) << " ";
            }
            std::cout << std::endl;
        }
        else if (m_bytesReceived == 0)
        {
            std::cout << "[INFO] [NET] Connection closed by client" << std::endl;
        }
        else
        {
            std::cout << "[ERROR] [NET] Error receiving data" << std::endl;
        }
    }
    
    /* Resize to actual bytes read */
    // m_tcpServerRx.resize(m_bytesRead);

    return m_bytesReceived;
}

int iceNET::closeCOM() 
{
    if (m_clientSocket >= 0) 
    {
        close(m_clientSocket);
        m_clientSocket = -1;
        m_clientConnected = false;
    }

    return 0;
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

void iceNET::killThread()
{
    m_killThread = true;
}

void iceNET::iceNETThread()
{
    std::cout << "[INFO] [NET] Enter iceNETThread" << std::endl;

    socklen_t clientLength = sizeof(m_clientAddress);

    while (!m_killThread)
    {
        std::cout << "[INFO] [NET] iceNETThread ready for next TCP packet" << std::endl;

        if (false == m_clientConnected)
        {
            /* Wait for the TCP client connection */
            m_clientSocket = accept(m_serverSocket, (struct sockaddr *)&m_clientAddress, &clientLength);

            if(dataRX() > 0)
            {
                /* TODO :: Set the flag to indicate data ready */
                setTcpRxReady(true);
            }
            else
            {
                std::cout << "[INFO] [NET] Nothing Received :: Cannot set TCP_TO_CHAR in State Machine" << std::endl;
            }

            if (dataTX() < 0)
            {
                std::cout << "[ERROR] [NET] Failed to send message" << std::endl;
            }
            else
            {
                std::cout << "[INFO] [NET] Transfer complete" << std::endl;
            }

            closeCOM();

        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [NET] Terminate iceNETThread" << std::endl;
}

/**
 * 
 * Data
 * 
 */
std::vector<char>* iceNET::getTcpServerRx()
{
    return &m_tcpServerRx;
}

void iceNET::setTcpServerTx(std::vector<char>* tcpVector)
{
    m_tcpServerTx = *tcpVector;
}

/**
 * 
 * Flags
 * 
 */
bool iceNET::getTcpRxReady()
{
    return m_tcpRxReady;
}

void iceNET::setTcpRxReady(bool flag)
{
    m_tcpRxReady = flag;
}

bool iceNET::getTcpTxReady()
{
    return m_tcpTxReady;
}

void iceNET::setTcpTxReady(bool flag)
{
    m_tcpTxReady = flag;
}