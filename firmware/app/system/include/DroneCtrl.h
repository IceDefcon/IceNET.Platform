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
        bool m_threadKill;

        /* Event Variables */
        std::mutex m_eventMutex;
        std::condition_variable m_conditionalVariable;
        bool m_stateChanged;

    public:
        DroneCtrl();
        ~DroneCtrl();

        /* THREAD */ void initThread();
        /* THREAD */ void shutdownThread();
        /* THREAD */ std::string getThreadStateMachineString(droneCtrlStateType state);
        /* THREAD */ void DroneCtrlThread();

        void initKernelComms();
        void shutdownKernelComms(bool isKernelConnected);
        bool isKilled();


        void sendFpgaConfigToRamDisk();
        void setDroneCtrlState(droneCtrlStateType state);

        /* EVENT */ void waitEvent();
        /* EVENT */ void triggerEvent();
};
