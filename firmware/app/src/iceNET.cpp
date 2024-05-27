/*!
 * 
 * Author: Ice.Marek and Ventor
 * IceNET Technology 2024
 * 
 */
#include "iceNET.h"
#include <iostream>
#include <cstring>
#include <unistd.h>

iceNET::iceNET(int portNumber):
portNumber(portNumber),
serverSocket(-1),
clientSocket(-1),
clientConnected(false) 
{
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portNumber);
}

iceNET::~iceNET() 
{
    std::cout << "[INFO] Destroying iceNET" << std::endl;
    closeClient();
    if (serverSocket >= 0) 
    {
        close(serverSocket);
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

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
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
    
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0) 
    {
        Console::Error("[NET] Error setting socket options");
    }

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) 
    {
        Console::Error("[NET] Error on binding the socket");
    }

    /* listen at the TCP Port */
    if (listen(serverSocket, 5) < 0) 
    {
        Console::Error("[NET] Error listening for connections");
    }

    while(!m_killThread) 
    {
        /* TODO */
    }

    Console::Info("[NET] Terminate iceNETThread");
}













bool iceNET::acceptClient() 
{
    socklen_t clientLength = sizeof(clientAddress);
    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLength);
    if (clientSocket < 0) 
    {
        perror("Socket Server: failed to accept the client connection");
        return false;
    }
    clientConnected = true;
    return true;
}

ssize_t iceNET::dataTX(const std::string& message) 
{
    if (!clientConnected) 
    {
        std::cerr << "Socket Server: no client connected" << std::endl;
        return -1;
    }
    return write(clientSocket, message.c_str(), message.length());
}

std::string iceNET::dataRX(size_t bufferSize) 
{
    if (!clientConnected) 
    {
        std::cerr << "Socket Server: no client connected" << std::endl;
        return "";
    }
    char buffer[bufferSize];
    ssize_t bytesRead = read(clientSocket, buffer, bufferSize);
    if (bytesRead < 0) 
    {
        perror("Socket Server: error reading from socket");
        return "";
    }
    return std::string(buffer, bytesRead);
}

void iceNET::closeClient() 
{
    if (clientSocket >= 0) 
    {
        close(clientSocket);
        clientSocket = -1;
        clientConnected = false;
    }
}
