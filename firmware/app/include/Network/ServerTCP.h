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

        int m_timeoutCount;
        int m_portNumber;
        int m_serverSocket;
        int m_clientSocket;
        bool m_clientConnected;

        struct sockaddr_in m_serverAddress;
        struct sockaddr_in m_clientAddress;

        std::shared_ptr<std::vector<char>> m_ServerTCPVector;

        std::vector<char>* m_Rx_ServerTCP;
        std::vector<char>* m_Tx_ServerTCP;
        int m_Rx_bytesReceived;
        int m_Tx_bytesReceived;

        bool m_transferComplete;

    public:
        ServerTCP();
        ~ServerTCP();

        void initBuffers();

        void initThread();
        void shutdownThread();
        bool isThreadKilled();
        void threadServerTCP();

        int configureServer();
        int tcpTX();
        int tcpRX();
        int tcpClose();

        /**
         * TODO
         *
         * This must be mutex protected
         * to avoid read/write in the
         * same time
         *
         */
        void setIO_State(ioStateType state);
        ioStateType getIO_State();

        /* COPY */ int getRx_ServerTCP(std::vector<char> &dataRx);
        /* COPY */ void setTx_ServerTCP(const std::vector<char> &dataTx, int size);

        /* SHARE */ void setSharedPointer(std::shared_ptr<std::vector<char>> sharedPointer);
};
