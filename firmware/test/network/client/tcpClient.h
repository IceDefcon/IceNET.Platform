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
        int socketfd;
        struct sockaddr_in serverAddress;
        struct hostent *server;
        std::string serverName;
        int portNumber;
        bool isConnected;

    public:
        tcpClient(const std::string &serverName, int portNumber);
        virtual int connectToServer();
        virtual int disconnectFromServer();
        virtual int send(const std::string &message);
        virtual std::string receive(int size = 1024);
        bool isClientConnected() const { return this->isConnected; }
        virtual ~tcpClient();
};
