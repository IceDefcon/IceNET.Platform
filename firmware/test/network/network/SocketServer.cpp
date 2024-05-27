#include "SocketServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

SocketServer::SocketServer(int portNumber) 
    : portNumber(portNumber), socketfd(-1), clientSocketfd(-1), clientConnected(false)
{
    bzero((char *)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portNumber);
}

int SocketServer::listen()
{
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        perror("Socket Server: error opening socket");
        return 1;
    }

    // Set socket options to reuse the address
    int opt = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) 
    {
        perror("Socket Server: error setting socket options");
        return 1;
    }

    if (bind(socketfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Socket Server: error on binding the socket");
        return 1;
    }

    ::listen(socketfd, 5);

    socklen_t clientLength = sizeof(clientAddress);
    clientSocketfd = accept(socketfd, (struct sockaddr *)&clientAddress, &clientLength);
    if (clientSocketfd < 0)
    {
        perror("Socket Server: failed to accept the client connection");
        return 1;
    }

    clientConnected = true;
    return 0;
}

int SocketServer::send(const std::string &message)
{
    if (!clientConnected)
    {
        std::cerr << "Socket Server: no client connected" << std::endl;
        return 1;
    }

    const char *writeBuffer = message.c_str();
    ssize_t n = write(clientSocketfd, writeBuffer, message.length());
    if (n < 0)
    {
        perror("Socket Server: error writing to socket");
        return 1;
    }
    return 0;
}

std::string SocketServer::receive(int size)
{
    if (!clientConnected)
    {
        std::cerr << "Socket Server: no client connected" << std::endl;
        return "";
    }

    char readBuffer[size];
    bzero(readBuffer, size);

    ssize_t n = read(clientSocketfd, readBuffer, size - 1);
    if (n < 0)
    {
        perror("Socket Server: error reading from socket");
        return "";
    }

    return std::string(readBuffer, n);
}

SocketServer::~SocketServer() 
{
    if (socketfd >= 0)
        close(socketfd);
    if (clientSocketfd >= 0)
        close(clientSocketfd);
}
