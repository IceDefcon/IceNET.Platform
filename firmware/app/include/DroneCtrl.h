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

        std::shared_ptr<std::vector<char>> m_Rx_DroneCtrlVector;
        std::shared_ptr<std::vector<char>> m_Tx_DroneCtrlVector;
        std::shared_ptr<ioStateType> m_IO_DroneCtrlState;

        std::vector<char>* m_Rx_DroneCtrl;
        std::vector<char>* m_Tx_DroneCtrl;
        int m_Rx_bytesReceived;
        int m_Tx_bytesReceived;

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
