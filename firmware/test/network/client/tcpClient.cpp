#include "tcpClient.h"
#include <iostream>
#include <cstring>
#include <unistd.h>

tcpClient::tcpClient(const std::string &serverName, int portNumber): 
m_serverName(serverName), 
m_portNumber(portNumber), 
m_socketfd(-1), 
m_server(nullptr), 
m_isConnected(false) 
{
    memset(&m_serverAddress, 0, sizeof(m_serverAddress));
    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_addr.s_addr = INADDR_ANY;
    m_serverAddress.sin_port = htons(m_portNumber);
}

int tcpClient::connectToServer()
{
    m_socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socketfd < 0)
    {
        perror("[NET] Socket Client: error opening socket");
        return 1;
    }

    m_server = gethostbyname(m_serverName.c_str());
    if (m_server == nullptr) 
    {
        std::cerr << "[NET] Socket Client: error - no such host" << std::endl;
        return 1;
    }

    if (connect(m_socketfd, (struct sockaddr *) &m_serverAddress, sizeof(m_serverAddress)) < 0)
    {
        perror("[NET] Socket Client: error connecting to the server");
        return 1;
    }

    m_isConnected = true;
    return 0;
}

int tcpClient::send(const std::string &message)
{
    if (!m_isConnected)
    {
        std::cerr << "[NET] Socket Client: not connected to server" << std::endl;
        return 1;
    }

    ssize_t n = write(m_socketfd, message.c_str(), message.length());
    if (n < 0)
    {
        perror("[NET] Socket Client: error writing to socket");
        return 1;
    }

    return 0;
}

std::string tcpClient::receive(int size)
{
    if (!m_isConnected)
    {
        std::cerr << "[NET] Socket Client: not connected to server" << std::endl;
        return "";
    }

    char readBuffer[size];
    bzero(readBuffer, size);

    ssize_t n = read(m_socketfd, readBuffer, size - 1);
    if (n < 0)
    {
        perror("[NET] Socket Client: error reading from socket");
        return "";
    }

    return std::string(readBuffer, n);
}

int tcpClient::disconnectFromServer()
{
    if (m_isConnected)
    {
        close(m_socketfd);
        m_isConnected = false;
    }
    return 0;
}

tcpClient::~tcpClient() 
{
    if (m_isConnected)
    {
        disconnectFromServer();
    }
}

bool tcpClient::isClientConnected()
{ 
    return m_isConnected; 
}
