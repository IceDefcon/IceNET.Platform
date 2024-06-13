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
#include <thread>
#include <atomic>
#include <vector>

#include "NetworkTraffic.h"

const size_t TCP_SERVER_SIZE = 32;

class ServerTCP
{
    private:
        int m_file_descriptor;
        std::thread m_threadServerTCP;
        std::atomic<bool> m_threadKill;

        int m_portNumber;
        int m_serverSocket;
        int m_clientSocket;
        int m_bytesReceived;
        bool m_clientConnected;

        struct sockaddr_in m_serverAddress;
        struct sockaddr_in m_clientAddress;

        /* For TCP server Traffic */
        std::vector<char>* m_ServerRx;
        std::vector<char>* m_ServerTx;

        std::shared_ptr<NetworkTraffic> m_instanceNetworkTraffic;

    public:
        ServerTCP();
        ~ServerTCP();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        void initThread();
        bool isThreadKilled();

        void threadServerTCP();

        int initServer();
        int tcpTX();
        int tcpRX();
        int tcpClose();

        void setInstance_NetworkTraffic(std::shared_ptr<NetworkTraffic> instance);
};
