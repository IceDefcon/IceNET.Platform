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
        bool m_isKernelConnected;

        /* Thread Variables */
        std::thread m_threadHandler;
        std::mutex m_threadMutex;
        std::mutex m_ctrlMutex;
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
        /* THREAD */ void setDroneCtrlState(droneCtrlStateType state);
        /* THREAD */ bool setKernelComms();
        /* THREAD */ void DroneCtrlThread();

        /* KERNEL */ int initKernelComms();
        /* KERNEL */ void shutdownKernelComms();
        /* KERNEL */ bool isKernelComsDead();

        void sendFpgaConfigToRamDisk();

        /* EVENT */ void waitDroneControlEvent();
        /* EVENT */ void triggerDroneControlEvent();

        /* GET */ bool getKernelConnected();
};
