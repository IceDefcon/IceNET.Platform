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
#include "Types.h"

class ServerTCP
{
    private:
        std::thread m_threadServerTCP;
        std::atomic<bool> m_threadKill;

        int m_timeoutCount;
        int m_portNumber;
        int m_serverSocket;
        int m_clientSocket;
        bool m_clientConnected;

        struct sockaddr_in m_serverAddress;
        struct sockaddr_in m_clientAddress;

        /* For TCP server Traffic */
        std::vector<char>* m_Rx_ServerTCP;
        std::vector<char>* m_Tx_ServerTCP;
        int m_Rx_bytesReceived;
        int m_Tx_bytesReceived;

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

        void setInstance_NetworkTraffic(const std::shared_ptr<NetworkTraffic> instance);
};
