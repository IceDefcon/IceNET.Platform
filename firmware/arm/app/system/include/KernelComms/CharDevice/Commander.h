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
#include <condition_variable>
#include <iostream>

#include "Measure.h"
#include "Types.h"

class Commander : public Measure
{
    private:
        int m_file_descriptor;

        /* Thread Variables */
        std::thread m_CommanderThread;
        std::mutex m_threadMutex;
        std::mutex m_ctrlMutex;
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

        /* Event Variables */
        std::mutex m_eventMutex;
        std::condition_variable m_conditionalVariable;
        bool m_stateChanged;
        uint8_t m_calibrationCount;

    public:
        Commander();
        ~Commander();

        void setupCommandMatrix();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        std::string getIoStateString(ioStateType state);
        std::string commandToString(commandType cmd);

        void setDmaCustom(uint8_t size);
        int sendCommand(commandType cmd);

        /* THREAD */ void initThread();
        /* THREAD */ void shutdownThread();
        /* THREAD */ bool isThreadKilled();
        /* THREAD */ void setCommanderState(ioStateType state);
        /* THREAD */ void CommanderThread();

        /* SHARE */ void setTransferPointers(
        std::shared_ptr<std::vector<uint8_t>> transferPointerRx,
        std::shared_ptr<std::vector<uint8_t>> transferPointerTx,
        std::shared_ptr<ioStateType> transferState);

        /* EVENT */ void waitCommanderEvent();
        /* EVENT */ void triggerCommanderEvent();
};
