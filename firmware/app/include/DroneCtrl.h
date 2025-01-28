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

#include "KernelComms.h"
#include "Network.h"
#include "Types.h"

class DroneCtrl :
    public Network,
    public KernelComms
{
    private:
        ioStateType m_ioState;
        ctrlType m_ctrlState;
        ctrlType m_ctrlStatePrev;

        Network::ServerTCP* m_instanceServerTCP;
        KernelComms::Commander* m_instanceCommander;
        KernelComms::Watchdog* m_instanceWatchdog;
        KernelComms::RamDisk* m_instanceRamDisk;

    public:
        DroneCtrl();
        ~DroneCtrl();

        void droneInit();
        void droneExit();
        bool isKilled();

        std::string getCtrlStateString(ctrlType state);

        void sendFpgaConfig();
        void droneCtrlMain();
};
