/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include "tcpServer.h"
#include <iostream>
#include <cstring>
#include <unistd.h>

tcpServer::tcpServer(int portNumber):
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

tcpServer::~tcpServer() 
{
    std::cout << "[INFO] Destroying tcpServer" << std::endl;
    closeClient();
    if (serverSocket >= 0) 
    {
        close(serverSocket);
    }
}

bool tcpServer::initialize() 
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) 
    {
        perror("Socket Server: error opening socket");
        return false;
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
        perror("Socket Server: error setting socket options");
        return false;
    }

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) 
    {
        perror("Socket Server: error on binding the socket");
        return false;
    }

    /* listen at the TCP Port */
    if (listen(serverSocket, 5) < 0) 
    {
        perror("Socket Server: error listening for connections");
        return false;
    }

    return true;
}

bool tcpServer::acceptClient() 
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

ssize_t tcpServer::dataTX(const std::string& message) 
{
    if (!clientConnected) 
    {
        std::cerr << "Socket Server: no client connected" << std::endl;
        return -1;
    }
    return write(clientSocket, message.c_str(), message.length());
}

std::string tcpServer::dataRX(size_t bufferSize) 
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

void tcpServer::closeClient() 
{
    if (clientSocket >= 0) 
    {
        close(clientSocket);
        clientSocket = -1;
        clientConnected = false;
    }
}
