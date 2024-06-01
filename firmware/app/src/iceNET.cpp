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
tcpServerRx(TCP_BUFFER_SIZE),
tcpServerTx(TCP_BUFFER_SIZE),
m_bytesRead(0)
{
    memset(&m_serverAddress, 0, sizeof(m_serverAddress));
    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_addr.s_addr = INADDR_ANY;
    m_serverAddress.sin_port = htons(m_portNumber);

    /* Initialize tcpServerRx and tcpServerTx with zeros */
    std::fill(tcpServerRx.begin(), tcpServerRx.end(), 0);
    std::fill(tcpServerTx.begin(), tcpServerTx.end(), 0);
}

iceNET::~iceNET() 
{
    Console::Info("[NET] Destroying iceNET");

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
        Console::Error("[NET] Error opening socket");
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
        Console::Error("[NET] Error setting socket options");
    }
    else
    {
        Console::Info("[NET] Socket option set correctly");
    }

    ret = bind(m_serverSocket, (struct sockaddr *)&m_serverAddress, sizeof(m_serverAddress));
    if (ret < 0) 
    {
        Console::Error("[NET] Error on binding the socket");
    }
    else
    {
        Console::Info("[NET] Socket bind successfully");
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
        Console::Error("[NET] Error listening for connections");
    }
    else
    {
        Console::Info("[NET] Listening at the TCP Port...");
        initThread();
    }

    return OK;
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
        tcpServerTx[0] = 0x69; /* i */
        tcpServerTx[1] = 0x63; /* c */
        tcpServerTx[2] = 0x65; /* e */
        tcpServerTx[3] = 0x4E; /* N */
        tcpServerTx[4] = 0x45; /* E */
        tcpServerTx[5] = 0x54; /* T */
        tcpServerTx[6] = '\n'; /* Next line for the GUI console */

        ret = write(m_clientSocket, tcpServerTx.data(), tcpServerTx.size());
    }

    return ret;
}

int iceNET::dataRX()
{
    if (!m_clientConnected) 
    {
        std::cerr << "Socket Server: no client connected" << std::endl;
        return ERROR;
    }
    
    m_bytesRead = read(m_clientSocket, tcpServerRx.data(), tcpServerRx.size());

    /**
     * 
     * TODO
     * 
     * m_killThread flag is not computed
     * 
     * This need to be done later 
     * 
     */
    if (m_bytesRead < 0) 
    {
        Console::Error("[NET] Error reading from socket");
        return ERROR;
    }

    /* Resize to actual bytes read */
    tcpServerRx.resize(m_bytesRead);

    return OK;
}

int iceNET::closeCOM() 
{
    if (m_clientSocket >= 0) 
    {
        close(m_clientSocket);
        m_clientSocket = -1;
        m_clientConnected = false;
    }

    return OK;
}

void iceNET::initThread()
{
    Console::Info("[NET] Init the iceNETThread");
    m_iceNETThread = std::thread(&iceNET::iceNETThread, this);
}

bool iceNET::isThreadKilled()
{
    return m_killThread;
}

void iceNET::iceNETThread()
{
    Console::Info("[NET] Enter iceNETThread");

    socklen_t clientLength = sizeof(m_clientAddress);

    while (!m_killThread)
    {
        Console::Info("[NET] iceNETThread ready for next TCP packet");

        if (false == m_clientConnected)
        {
            /* Wait for the TCP client connection */
            m_clientSocket = accept(m_serverSocket, (struct sockaddr *)&m_clientAddress, &clientLength);

            if (m_clientSocket < 0)
            {
                Console::Error("[NET] Failed to accept the client connection");
            }
            else
            {
                Console::Info("[NET] Client connected to server");
                m_clientConnected = true;

                int bytesReceived = recv(m_clientSocket, tcpServerRx.data(), TCP_BUFFER_SIZE, 0);

                /**
                 * 
                 * Here we need a signaling for the char Device
                 * to get data obtained from the TCP client
                 * 
                 */

                if (bytesReceived > 0)
                {
                    std::cout << "[INFO] [NET] Received " << bytesReceived << " Bytes of data: ";
                    for (int i = 0; i < bytesReceived; ++i)
                    {
                        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(tcpServerRx.data()[i]) << " ";
                    }
                    std::cout << std::endl;
                }
                else if (bytesReceived == 0)
                {
                    Console::Info("[NET] Connection closed by client");
                }
                else
                {
                    Console::Error("[NET] Error receiving data");
                }

                if (dataTX() < 0)
                {
                    Console::Error("[NET] Failed to send message");
                }
                else
                {
                    Console::Info("[NET] Transfer complete");
                }

                closeCOM();
            }
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Console::Info("[NET] Terminate iceNETThread");
}

std::vector<char>* iceNET::GET_tcpServerRx()
{
    return &tcpServerRx;
}

void iceNET::SET_tcpServerTx(std::vector<char>* tcpVector)
{
    tcpServerTx = *tcpVector;
}