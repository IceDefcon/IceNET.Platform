/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
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

#include "Commander.h"
#include "RamDisk.h"
#include "Types.h"

class ServerTCP
{
    private:
        std::thread m_threadServerTCP;
        std::atomic<bool> m_threadKill;

        ioStateType m_ioState;
        ioStateType m_ioStatePrev;

        int m_timeoutConnection;
        int m_timeoutTransfer;
        int m_portNumber;
        int m_serverSocket;
        int m_clientSocket;
        bool m_clientConnected;

        struct sockaddr_in m_serverAddress;
        struct sockaddr_in m_clientAddress;

        std::shared_ptr<std::vector<char>> m_Rx_ServerTCPVector;
        std::shared_ptr<std::vector<char>> m_Tx_ServerTCPVector;
        std::shared_ptr<ioStateType> m_IO_ServerTCPState;

    public:
        ServerTCP();
        ~ServerTCP();

        int configureServer();
        int tcpTX();
        int tcpRX();
        int tcpClose();

        void initThread();
        void shutdownThread();
        bool isThreadKilled();
        bool isClientConnected();
        void threadServerTCP();

        /* SHARE */ void setTransferPointer(std::shared_ptr<std::vector<char>> transferPointerRx, std::shared_ptr<std::vector<char>> transferPointerTx);
        /* SHARE */ void setTransferState(std::shared_ptr<ioStateType> transferStatee);
};
