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
#include <mutex>

#include "Types.h"

class Commander
{
    private:
        int m_file_descriptor;
        std::thread m_threadCommander;
        std::mutex m_threadMutex;
        bool m_threadKill;

        ioStateType m_ioState;
        ioStateType m_ioStatePrev;

        std::shared_ptr<std::vector<uint8_t>> m_Rx_CommanderVector;
        std::shared_ptr<std::vector<uint8_t>> m_Tx_CommanderVector;
        std::shared_ptr<ioStateType> m_IO_CommanderState;

        std::vector<std::vector<uint8_t>> m_commandMatrix;

        uint8_t m_customDmaSize;

        int16_t m_x;
        int16_t m_y;
        int16_t m_z;

        int16_t m_x_vector[32];
        int16_t m_y_vector[32];
        int16_t m_z_vector[32];

        int16_t m_x_average;
        int16_t m_y_average;
        int16_t m_z_average;

        struct timespec m_ts;
        double m_seconds_since_boot;

    public:
        Commander();
        ~Commander();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        std::string commandToString(commandType cmd);

        void setDmaCustom(uint8_t size);
        int sendCommand(commandType cmd);

        void initThread();
        void shutdownThread();
        bool isThreadKilled();
        void threadCommander();

        /* SHARE */ void setTransferPointers(
        std::shared_ptr<std::vector<uint8_t>> transferPointerRx,
        std::shared_ptr<std::vector<uint8_t>> transferPointerTx,
        std::shared_ptr<ioStateType> transferState);
};
