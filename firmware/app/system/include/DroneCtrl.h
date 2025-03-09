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
#include "Types.h"

class DroneCtrl :
    public KernelComms
{
    private:
        ctrlType m_ctrlState;
        ctrlType m_ctrlStatePrev;

        KernelComms::Commander* m_instanceCommander;
        KernelComms::Watchdog* m_instanceWatchdog;
        KernelComms::RamDisk* m_instanceRamDisk;

    public:
        DroneCtrl();
        ~DroneCtrl();

        void initPointers();
        void initKernelComms();
        void shutdownKernelComms();
        bool isKilled();

        std::string getCtrlStateString(ctrlType state);

        void sendFpgaConfig();
        void droneCtrlMain();

        Commander* getCommanderInstance();
};
