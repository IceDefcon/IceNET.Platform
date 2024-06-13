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

const size_t ICE_NET_BUFFER_SIZE = 32;

class outputCOM
{
    private:
        int m_file_descriptor;
        std::thread m_outputCOMThread;
        std::atomic<bool> m_killThread;

        int m_portNumber;
        int m_serverSocket;
        int m_clientSocket;
        int m_bytesReceived;
        bool m_clientConnected;

        struct sockaddr_in m_serverAddress;
        struct sockaddr_in m_clientAddress;

        /* For TCP server Traffic */
        std::vector<char>* m_outputCOMRx;
        std::vector<char>* m_outputCOMTx;

        linkCOM* m_linkCOMIstance;

    public:
        outputCOM();
        ~outputCOM();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        void initThread();
        bool isThreadKilled();

        void outputCOMThread();

        void setlinkCOMIstance(linkCOM* instance);
};
