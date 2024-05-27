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

class iceNET 
{
    private:
        int portNumber;
        int serverSocket;
        int clientSocket;
        struct sockaddr_in serverAddress;
        struct sockaddr_in clientAddress;
        bool clientConnected;

        std::thread m_iceNETThread;

        /*!
         * 
         * Atomic in case if something 
         * decide to interrupt kill 
         * flag processing
         * 
         */
        std::atomic<bool> m_killThread;

    public:
        iceNET(int portNumber);
        ~iceNET();

        void initThread();
        void iceNetThread();

        bool acceptClient();
        ssize_t dataTX(const std::string& message);
        std::string dataRX(size_t bufferSize = 1024);
        void closeClient();
};
