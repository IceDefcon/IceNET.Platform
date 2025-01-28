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
        ctrlType m_droneCtrlState;

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

        void sendFpgaConfig();
        void droneCtrlMain();
};
