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

#include "RamConfig.h"
#include "Commander.h"
#include "Types.h"

class Watchdog
{
    private:
        int m_file_descriptor;
        std::thread m_threadWatchdog;
        std::atomic<bool> m_threadKill;

        bool m_stopFlag;

        std::vector<char>* m_Rx_Watchdog;
        std::vector<char>* m_Tx_Watchdog;

        RamConfig* m_instanceRamConfig;
        Commander* m_instanceCommander;

    public:
        Watchdog();
        ~Watchdog();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        void initThread();
        bool isThreadKilled();

        void threadWatchdog();

        void setInstance_RamConfig(RamConfig* instance);
        void setInstance_Commander(Commander* instance);

};
