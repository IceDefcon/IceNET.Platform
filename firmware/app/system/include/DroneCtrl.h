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

class DroneCtrl : public KernelComms
{
    private:
        droneCtrlStateType m_ctrlState;
        droneCtrlStateType m_ctrlStatePrev;

        static constexpr uint32_t FPGA_DELAY = 3000;

        /* Thread Variables */
        std::thread m_threadHandler;
        std::mutex m_threadMutex;
        std::mutex m_ctrlMutex;
        bool m_threadKill;

        /* Event Variables */
        std::mutex m_eventMutex;
        std::condition_variable m_conditionalVariable;
        bool m_stateChanged;

        std::shared_ptr<std::vector<uint8_t>> m_Rx_DroneCtrlVector;
        std::shared_ptr<std::vector<uint8_t>> m_Tx_DroneCtrlVector;
        std::shared_ptr<ioStateType> m_IO_DroneCtrlState;

    public:
        DroneCtrl();
        ~DroneCtrl();

        /* THREAD */ void initThread();
        /* THREAD */ void shutdownThread();
        /* THREAD */ std::string getThreadStateMachineString(droneCtrlStateType state);
        /* THREAD */ void setDroneCtrlState(droneCtrlStateType state);
        /* THREAD */ void DroneCtrlThread();

        void initKernelComms();
        void shutdownKernelComms(bool isKernelConnected);
        bool isKilled();

        void sendFpgaConfigToRamDisk();

        /* EVENT */ void waitEvent();
        /* EVENT */ void triggerEvent();
};
