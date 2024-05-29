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

class iceNET : public Core
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

    public:
        iceNET(int portNumber);
        ~iceNET();

        void initThread();
        void iceNETThread();

        int openCOM(const char* device) override;
        int dataTX() override;
        int dataRX() override;
        int closeCOM() override;

        ssize_t dataTX(const std::string& message);
        std::string dataRX(size_t bufferSize = 1024);
        void closeClient();
};
