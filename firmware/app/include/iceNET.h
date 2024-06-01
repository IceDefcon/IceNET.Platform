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

#include "core.h"
#include "console.h"
#include "compute.h"

const size_t TCP_BUFFER_SIZE = 32;

class iceNET : public Core, public Compute
{
    private:
        int m_portNumber;
        int m_serverSocket;
        int m_clientSocket;
        bool m_clientConnected;
        std::thread m_iceNETThread;
        std::atomic<bool> m_killThread;

        struct sockaddr_in m_serverAddress;
        struct sockaddr_in m_clientAddress;

        /* For TCP server Traffic */
        std::vector<char> tcpServerRx;
        std::vector<char> tcpServerTx;

        ssize_t m_bytesRead;

    public:
        iceNET(int portNumber);
        ~iceNET();

        int openCOM() override;
        int dataTX() override;
        int dataRX() override;
        int closeCOM() override;

        void initThread() override;
        bool isThreadKilled() override;

        void iceNETThread();

        std::vector<char>* GET_tcpServerRx();
        void SET_tcpServerTx(std::vector<char>* tcpVector);
};
