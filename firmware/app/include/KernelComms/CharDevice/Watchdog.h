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

#include "RamDisk.h"
#include "Commander.h"
#include "Types.h"

class Watchdog
{
    private:
        int m_file_descriptor;
        std::thread m_threadWatchdog;
        std::atomic<bool> m_threadKill;

        bool m_stopFlag;
        bool m_watchdogDead;

        std::vector<char>* m_Rx_Watchdog;
        std::vector<char>* m_Tx_Watchdog;

        RamDisk* m_instanceRamDisk;
        Commander* m_instanceCommander;

    public:
        Watchdog();
        ~Watchdog();

        void initBuffers();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        void initThread();
        void shutdownThread();
        bool isThreadKilled();
        void threadWatchdog();

        bool isWatchdogDead();

        void setRamDiskInstance(RamDisk* instance);
        void setCommanderInstance(Commander* instance);

};
