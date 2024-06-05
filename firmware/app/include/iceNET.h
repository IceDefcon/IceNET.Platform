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

const size_t TCP_BUFFER_SIZE = 32;

class iceNET
{
    private:
        int m_portNumber;
        int m_serverSocket;
        int m_clientSocket;
        int m_bytesReceived;
        bool m_clientConnected;
        std::thread m_iceNETThread;
        std::atomic<bool> m_killThread;

        struct sockaddr_in m_serverAddress;
        struct sockaddr_in m_clientAddress;

        /* For TCP server Traffic */
        std::vector<char> m_tcpServerRx;
        std::vector<char> m_tcpServerTx;

        /* Data ready :: For SM */
        bool m_tcpRxReady;
        bool m_tcpTxReady;

        ssize_t m_bytesRead;

    public:
        iceNET(int portNumber);
        ~iceNET();

        int openCOM();
        int dataTX();
        int dataRX();
        int closeCOM();

        void initThread();
        bool isThreadKilled();
        void killThread();

        void iceNETThread();

        std::vector<char>* getTcpServerRx();
        void setTcpServerTx(std::vector<char>* tcpVector);

        bool getTcpRxReady();
        void setTcpRxReady(bool flag);
        bool getTcpTxReady();
        void setTcpTxReady(bool flag);


};
