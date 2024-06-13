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

#include "linkCOM.h"

const size_t TCP_BUFFER_SIZE = 32;

class serverTCP
{
    private:
        int m_file_descriptor;
        std::thread m_serverTCPThread;
        std::atomic<bool> m_killThread;

        int m_portNumber;
        int m_serverSocket;
        int m_clientSocket;
        int m_bytesReceived;
        bool m_clientConnected;

        struct sockaddr_in m_serverAddress;
        struct sockaddr_in m_clientAddress;

        /* For TCP server Traffic */
        std::vector<char>* m_serverTCPRx;
        std::vector<char>* m_serverTCPTx;

        linkCOM* m_linkCOMIstance;

    public:
        serverTCP();
        ~serverTCP();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        void initThread();
        bool isThreadKilled();

        void serverTCPThread();

        int initServer();
        int tcpTX();
        int tcpRX();
        int tcpClose();

        void setlinkCOMIstance(linkCOM* instance);
};
