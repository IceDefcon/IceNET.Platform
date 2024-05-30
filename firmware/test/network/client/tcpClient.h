/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>

class tcpClient 
{
    private:
        std::string m_serverName;
        int m_portNumber;
        int m_socketfd;
        struct sockaddr_in m_serverAddress;
        struct hostent *m_server;
        bool m_isConnected;

    public:
        tcpClient(const std::string &serverName, int portNumber);
        virtual int connectToServer();
        virtual int disconnectFromServer();
        virtual int send(const std::string &message);
        virtual std::string receive(int size = 1024);
        bool isClientConnected();
        virtual ~tcpClient();
};
