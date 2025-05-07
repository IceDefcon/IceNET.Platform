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
#include <mutex>

#include "RamDisk.h"
#include "Commander.h"
#include "Types.h"

#define WATCHDOG_TRANSFER_SIZE 2

class Watchdog
{
    private:
        int m_file_descriptor;
        std::thread m_threadWatchdog;
        std::mutex m_threadMutex;
        bool m_threadKill;

        bool m_stopFlag;
        bool m_fpgaConfigReady;
        bool m_watchdogDead;

        std::vector<char>* m_Rx_Watchdog;
        std::vector<char>* m_Tx_Watchdog;

    public:
        Watchdog();
        ~Watchdog();

        void initBuffers();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        void initThread();
        void shutdownThread(bool isKernelConnected);
        bool isThreadKilled();
        void threadWatchdog();

        bool isWatchdogDead();

        void setFpgaConfigReady();
        bool getFpgaConfigReady();
};
