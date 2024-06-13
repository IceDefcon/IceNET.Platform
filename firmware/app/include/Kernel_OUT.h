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

const size_t ICE_NET_BUFFER_SIZE = 32;

class Kernel_OUT
{
    private:
        int m_file_descriptor;
        std::thread m_Kernel_OUTThread;
        std::atomic<bool> m_killThread;

        int m_portNumber;
        int m_serverSocket;
        int m_clientSocket;
        int m_bytesReceived;
        bool m_clientConnected;

        struct sockaddr_in m_serverAddress;
        struct sockaddr_in m_clientAddress;

        /* For TCP server Traffic */
        std::vector<char>* m_Kernel_OUTRx;
        std::vector<char>* m_Kernel_OUTTx;

        NetworkTraffic* m_NetworkTrafficIstance;

    public:
        Kernel_OUT();
        ~Kernel_OUT();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        void initThread();
        bool isThreadKilled();

        void Kernel_OUTThread();

        void setNetworkTrafficIstance(NetworkTraffic* instance);
};
