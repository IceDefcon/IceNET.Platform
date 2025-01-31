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

        std::shared_ptr<std::vector<char>> m_Rx_CommanderVector;
        std::shared_ptr<std::vector<char>> m_Tx_CommanderVector;
        std::shared_ptr<ioStateType> m_IO_CommanderState;

        std::vector<std::vector<char>> m_commandMatrix;

    public:
        Commander();
        ~Commander();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        void sendCommand();
        int activateConfig();
        void reconfigureEngine();

        void initThread();
        void shutdownThread();
        bool isThreadKilled();
        void threadCommander();

        /* SHARE */ void setTransferPointer(std::shared_ptr<std::vector<char>> transferPointerRx, std::shared_ptr<std::vector<char>> transferPointerTx);
        /* SHARE */ void setTransferState(std::shared_ptr<ioStateType> transferStatee);
};
