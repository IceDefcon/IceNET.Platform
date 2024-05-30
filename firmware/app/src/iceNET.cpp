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
#include <unistd.h>
#include "iceNET.h"

iceNET::iceNET(int portNumber):
    m_portNumber(portNumber),
    m_serverSocket(-1),
    m_clientSocket(-1),
    m_clientConnected(false),
    m_iceNETThread(),
    m_killThread(false)
{
    memset(&m_serverAddress, 0, sizeof(m_serverAddress));
    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_addr.s_addr = INADDR_ANY;
    m_serverAddress.sin_port = htons(m_portNumber);
}

iceNET::~iceNET() 
{
    Console::Info("[NET] Destroying iceNET");

    closeClient();
    
    if (m_serverSocket >= 0) 
    {
        close(m_serverSocket);
    }

    if (m_iceNETThread.joinable()) 
    {
        m_iceNETThread.join();
    }
}

void iceNET::initThread()
{
    Console::Info("[NET] Init the iceNETThread");
    m_iceNETThread = std::thread(&iceNET::iceNETThread, this);
}

void iceNET::iceNETThread()
{
    Console::Info("[NET] Enter iceNETThread");
    
    socklen_t clientLength = sizeof(m_clientAddress);

    while(!m_killThread) 
    {
        Console::Info("[NET] iceNETThread ready for next TCP packet");

        if(false == m_clientConnected)
        {
            /* Wait for the TCP client connection */
            m_clientSocket = accept(m_serverSocket, (struct sockaddr *)&m_clientAddress, &clientLength);
            
            if (m_clientSocket < 0) 
            {
                Console::Error("[NET] Failed to accept the client connection");
            }
            else
            {
                m_clientConnected = true;

                std::string receivedMessage = dataRX(1024);
                if (receivedMessage.empty()) 
                {
                    std::cerr << "[NET] Failed to receive message" << std::endl;
                }

                std::cout << "[INFO] [NET] Server RX :: " << receivedMessage << std::endl;

                std::string responseMessage("[SERVER] <--- [CLIENT]");
                std::cout << "[INFO] [NET] Server TX :: " << responseMessage << std::endl;

                if (dataTX(responseMessage) < 0) 
                {
                    std::cerr << "[NET] Failed to send message" << std::endl;
                }
                else
                {
                    Console::Info("[NET] Transfer complete");
                    closeClient();
                }
            }
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Console::Info("[NET] Terminate iceNETThread");
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

ssize_t iceNET::dataTX(const std::string& message) 
{
    if (!m_clientConnected) 
    {
        std::cerr << "Socket Server: no client connected" << std::endl;
        return -1;
    }
    return write(m_clientSocket, message.c_str(), message.length());
}

std::string iceNET::dataRX(size_t bufferSize) 
{
    if (!m_clientConnected) 
    {
        std::cerr << "Socket Server: no client connected" << std::endl;
        return "";
    }
    char buffer[bufferSize];
    ssize_t bytesRead = read(m_clientSocket, buffer, bufferSize);
    if (bytesRead < 0) 
    {
        Console::Error("[NET] Error reading from socket");
        return "";
    }
    return std::string(buffer, bytesRead);
}

void iceNET::closeClient() 
{
    if (m_clientSocket >= 0) 
    {
        close(m_clientSocket);
        m_clientSocket = -1;
        m_clientConnected = false;
    }
}

int iceNET::dataTX() {return 0;}
int iceNET::dataRX() {return 0;}
int iceNET::closeCOM() {return 0;}

bool iceNET::terminate()
{
    return m_killThread;
}
