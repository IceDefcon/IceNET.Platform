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
#include <string>

class tcpServer {
private:
    int portNumber;
    int serverSocket;
    int clientSocket;
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    bool clientConnected;

public:
    tcpServer(int portNumber);
    ~tcpServer();

    bool initialize();
    bool acceptClient();
    ssize_t dataTX(const std::string& message);
    std::string dataRX(size_t bufferSize = 1024);
    void closeClient();
};
