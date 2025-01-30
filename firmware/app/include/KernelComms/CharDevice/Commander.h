/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <vector>

#include "Types.h"

class Commander
{
    private:
        int m_file_descriptor;
        std::thread m_threadCommander;
        std::atomic<bool> m_threadKill;

        ioStateType m_ioState;
        ioStateType m_ioStatePrev;

        Commander* m_instance;

        std::vector<char>* m_Rx_Commander;
        std::vector<char>* m_Tx_Commander;
        int m_Rx_bytesReceived;
        int m_Tx_bytesReceived;

        bool m_transferComplete;

    public:
        Commander();
        ~Commander();

        void initBuffers();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        void initThread();
        void shutdownThread();
        bool isThreadKilled();
        void threadCommander();

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

        /* COPY */ int getRx_Commander(std::vector<char> &dataRx);
        /* COPY */ void setTx_Commander(const std::vector<char> &dataTx, int size);
};
