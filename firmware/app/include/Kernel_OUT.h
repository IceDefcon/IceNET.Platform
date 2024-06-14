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

const size_t KERNEL_IN_SIZE = 32;

class Kernel_OUT
{
    private:
        int m_file_descriptor;
        std::thread m_Kernel_OUTThread;
        std::atomic<bool> m_threadKill;

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

        std::shared_ptr<NetworkTraffic> m_instanceNetworkTraffic;

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

        void setInstance_NetworkTraffic(std::shared_ptr<NetworkTraffic> instance);
};
