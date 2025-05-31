/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "KernelCtrl.h"

KernelCtrl::KernelCtrl() :
    m_ctrlState(MAIN_CTRL_IDLE),
    m_ctrlStatePrev(MAIN_CTRL_IDLE),
    m_isKernelConnected(false)
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate KernelCtrl" << std::endl;

    initThread();
}

KernelCtrl::~KernelCtrl()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy KernelCtrl" << std::endl;

    shutdownThread();

    m_isKernelConnected = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* THREAD */ std::string KernelCtrl::getThreadStateMachineString(KernelCtrlStateType state)
{
    static const std::array<std::string, MAIN_CTRL_AMOUNT> ctrlStateStrings =
    {
        "MAIN_CTRL_IDLE",
        "MAIN_CTRL_INIT",
        "MAIN_CTRL_CONFIG",
    };

    if (state >= 0 && state < MAIN_CTRL_AMOUNT)
    {
        return ctrlStateStrings[state];
    }
    else
    {
        return "UNKNOWN_STATE";
    }
}

/* THREAD */ void KernelCtrl::setKernelCtrlState(KernelCtrlStateType state)
{
    std::unique_lock<std::mutex> lock(m_ctrlMutex);
    m_ctrlState = state;
    triggerKernelCtrlEvent();
}

/* THREAD */ void KernelCtrl::KernelCtrlThread()
{
    while (!m_threadKill)
    {
        if(m_ctrlStatePrev != m_ctrlState)
        {
            std::cout << "[INFO] [ D ] State KernelCtrl " << m_ctrlStatePrev << "->" << m_ctrlState << " " << getThreadStateMachineString(m_ctrlState) << std::endl;
            m_ctrlStatePrev = m_ctrlState;
        }

        switch(m_ctrlState)
        {
            case MAIN_CTRL_IDLE:
                waitKernelCtrlEvent();
                break;

            case MAIN_CTRL_INIT:
                break;

            case MAIN_CTRL_CONFIG:
                break;

            default:
                std::cout << "[INFO] [ D ] Unknown State" << std::endl;
        }
    }

    std::cout << "[INFO] [ D ] Terminate KernelCtrlThread" << std::endl;
}

/* THREAD */ void KernelCtrl::initThread()
{
    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadHandler.joinable())
    {
        std::cout << "[INFO] [ D ] KernelCtrlThread is already running" << std::endl;
        return;
    }

    m_threadKill = false;
    m_threadHandler = std::thread(&KernelCtrl::KernelCtrlThread, this);
    std::cout << "[INFO] [ D ] KernelCtrlThread Initialized" << std::endl;
}

/* THREAD */ void KernelCtrl::shutdownThread()
{
    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadKill)
    {
        std::cout << "[INFO] [ D ] KernelCtrlThread is already marked for shutdown" << std::endl;
        return;
    }

    m_threadKill = true;
    triggerKernelCtrlEvent();

    if (m_threadHandler.joinable())
    {
        m_threadHandler.join();
    }

    std::cout << "[INFO] [ D ] KernelCtrlThread Terminated" << std::endl;
}

/* EVENT */ void KernelCtrl::waitKernelCtrlEvent()
{
    std::cout << "[INFO] [ D ] KernelCtrlThread Wait" << std::endl;
    std::unique_lock<std::mutex> lock(m_eventMutex);

    auto predicate = [this]()
    {
        return m_stateChanged;
    };

    m_conditionalVariable.wait(lock, predicate);
    m_stateChanged = false;
}

/* EVENT */ void KernelCtrl::triggerKernelCtrlEvent()
{
    std::cout << "[INFO] [ D ] KernelCtrlThread Event" << std::endl;
    std::lock_guard<std::mutex> lock(m_eventMutex);

    m_stateChanged = true;
    m_conditionalVariable.notify_one();
}

/* GET */ bool KernelCtrl::getKernelConnected()
{
    return m_isKernelConnected;
}
