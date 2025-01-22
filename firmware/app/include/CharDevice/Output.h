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

class Output
{
    private:
        int m_file_descriptor;
        std::thread m_threadOutput;
        std::atomic<bool> m_threadKill;

        int m_portNumber;
        int m_serverSocket;
        int m_clientSocket;
        int m_bytesReceived;
        bool m_clientConnected;

        struct sockaddr_in m_serverAddress;
        struct sockaddr_in m_clientAddress;

        /* For TCP server Traffic */
        std::vector<char>* m_Rx_Output;
        std::vector<char>* m_Tx_Output;

        std::shared_ptr<NetworkTraffic> m_instanceNetworkTraffic;

    public:
        Output();
        ~Output();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        void initThread();
        bool isThreadKilled();

        void threadOutput();

        void setInstance_NetworkTraffic(const std::shared_ptr<NetworkTraffic> instance);
};
