/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#pragma once

#include <condition_variable>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <iomanip>
#include <fcntl.h>
#include <chrono>
#include <thread>
#include <string>
#include <atomic>
#include <vector>
#include <mutex>
#include <array>

#include "KernelCommander.h"

typedef enum
{
    KERNEL_CTRL_IDLE = 0,
    KERNEL_CTRL_INIT,
    KERNEL_CTRL_CONFIG,
    KERNEL_CTRL_MAIN,
    KERNEL_CTRL_AMOUNT,
} KernelCtrlStateType;

class KernelCtrl : public KernelCommander
{
    private:

        /* Main Variables */
        KernelCtrlStateType m_ctrlState;
        KernelCtrlStateType m_ctrlStatePrev;
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
        KernelCtrl();
        ~KernelCtrl();

        /* THREAD */ void initThread();
        /* THREAD */ void shutdownThread();
        /* THREAD */ std::string getThreadStateMachineString(KernelCtrlStateType state);
        /* THREAD */ void setKernelCtrlState(KernelCtrlStateType state);
        /* THREAD */ void KernelCtrlThread();

        /* COMMS */ int initKernelComms();
        /* COMMS */ void shutdownKernelComms();
        /* COMMS */ bool isKernelComsDead();

        /* EVENT */ void waitKernelCtrlEvent();
        /* EVENT */ void triggerKernelCtrlEvent();

        /* GET */ bool getKernelConnected();
};
