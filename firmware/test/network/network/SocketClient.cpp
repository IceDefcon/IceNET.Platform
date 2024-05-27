#include "SocketClient.h"
#include <iostream>
#include <cstring>
#include <unistd.h>

SocketClient::SocketClient(const std::string &serverName, int portNumber) : 
    serverName(serverName), 
    portNumber(portNumber), 
    socketfd(-1), server(nullptr), 
    isConnected(false) 
    {
        /* TODO */
    }

int SocketClient::connectToServer()
{
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        perror("Socket Client: error opening socket");
        return 1;
    }

    server = gethostbyname(serverName.c_str());
    if (server == nullptr) 
    {
        std::cerr << "Socket Client: error - no such host" << std::endl;
        return 1;
    }

    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(portNumber);

    if (connect(socketfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Socket Client: error connecting to the server");
        return 1;
    }

    isConnected = true;
    return 0;
}

int SocketClient::send(const std::string &message)
{
    if (!isConnected)
    {
        std::cerr << "Socket Client: not connected to server" << std::endl;
        return 1;
    }

    ssize_t n = write(this->socketfd, message.c_str(), message.length());
    if (n < 0)
    {
        perror("Socket Client: error writing to socket");
        return 1;
    }

    return 0;
}

std::string SocketClient::receive(int size)
{
    if (!isConnected)
    {
        std::cerr << "Socket Client: not connected to server" << std::endl;
        return "";
    }

    char readBuffer[size];
    bzero(readBuffer, size);

    ssize_t n = read(this->socketfd, readBuffer, size - 1);
    if (n < 0)
    {
        perror("Socket Client: error reading from socket");
        return "";
    }

    return std::string(readBuffer, n);
}

int SocketClient::disconnectFromServer()
{
    if (isConnected)
    {
        close(this->socketfd);
        isConnected = false;
    }
    return 0;
}

SocketClient::~SocketClient() 
{
    if (isConnected)
    {
        disconnectFromServer();
    }
}
