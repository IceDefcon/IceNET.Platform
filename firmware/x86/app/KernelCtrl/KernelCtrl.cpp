/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "KernelCtrl.h"

KernelCtrl::KernelCtrl() :
m_ctrlState(KERNEL_CTRL_IDLE),
m_ctrlStatePrev(KERNEL_CTRL_IDLE),
m_isKernelConnected(false)
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate KernelCtrl" << std::endl;

    initThread();
}

KernelCtrl::~KernelCtrl()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy KernelCtrl" << std::endl;

    shutdownKernelComms();
    shutdownThread();

    m_isKernelConnected = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* THREAD */ void KernelCtrl::initThread()
{
    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadHandler.joinable())
    {
        std::cout << "[INFO] [ K ] KernelCtrlThread is already running" << std::endl;
        return;
    }

    m_threadKill = false;
    m_threadHandler = std::thread(&KernelCtrl::KernelCtrlThread, this);
    std::cout << "[INFO] [ K ] KernelCtrlThread Initialized" << std::endl;
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
        std::cout << "[INFO] [ K ] KernelCtrlThread is already marked for shutdown" << std::endl;
        return;
    }

    m_threadKill = true;
    triggerKernelCtrlEvent();

    if (m_threadHandler.joinable())
    {
        m_threadHandler.join();
    }

    std::cout << "[INFO] [ K ] KernelCtrlThread Terminated" << std::endl;
}

/* THREAD */ std::string KernelCtrl::getThreadStateMachineString(KernelCtrlStateType state)
{
    static const std::array<std::string, KERNEL_CTRL_AMOUNT> ctrlStateStrings =
    {
        "KERNEL_CTRL_IDLE",
        "KERNEL_CTRL_INIT",
        "KERNEL_CTRL_CONFIG",
        "KERNEL_CTRL_MAIN",
    };

    if (state >= 0 && state < KERNEL_CTRL_AMOUNT)
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
            std::cout << "[INFO] [ K ] State KernelCtrl " << m_ctrlStatePrev << "->" << m_ctrlState << " " << getThreadStateMachineString(m_ctrlState) << std::endl;
            m_ctrlStatePrev = m_ctrlState;
        }

        switch(m_ctrlState)
        {
            case KERNEL_CTRL_IDLE:
                waitKernelCtrlEvent();
                break;

            case KERNEL_CTRL_INIT:
                m_isKernelConnected = (OK == initKernelComms()) ? true : false;
                if(false == m_isKernelConnected)
                {
                    std::cout << "[INFO] [ K ] Kernel Connection -> Failure" << std::endl;
                    m_ctrlState = KERNEL_CTRL_IDLE;
                    break;
                }
                else
                {
                    std::cout << "[INFO] [ K ] Kernel Connection -> Success" << std::endl;
                }
                m_ctrlState = KERNEL_CTRL_CONFIG;
                break;

            case KERNEL_CTRL_CONFIG:
                m_ctrlState = KERNEL_CTRL_IDLE;
                break;

            case KERNEL_CTRL_MAIN:
                //
                // TODO
                //
                break;

            default:
                std::cout << "[INFO] [ K ] Unknown State" << std::endl;
        }
    }

    std::cout << "[INFO] [ K ] Terminate KernelCtrlThread" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* COMMS */ int KernelCtrl::initKernelComms()
{
    int ret;

    std::cout << "[INIT] [ K ] Initialize KernelCommander" << std::endl;

    ret = KernelCommander::openDEV();
    if(OK != ret)
    {
        std::cout << "[ERNO] [ K ] KernelCommander Problem -> " << ret << std::endl;
        return ret;
    }

    return ret;
}

/* COMMS */ void KernelCtrl::shutdownKernelComms()
{
    std::cout << "[INFO] [ K ] Shutdown KernelCommander" << std::endl;
    KernelCommander::shutdownThread();
}

/* COMMS */ bool KernelCtrl::isKernelComsDead()
{
    bool ret = false;

    ret = KernelCommander::isThreadKilled();

    return ret;
}

/* EVENT */ void KernelCtrl::waitKernelCtrlEvent()
{
    std::cout << "[INFO] [ K ] KernelCtrlThread Wait" << std::endl;
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
    std::cout << "[INFO] [ K ] KernelCtrlThread Event" << std::endl;
    std::lock_guard<std::mutex> lock(m_eventMutex);

    m_stateChanged = true;
    m_conditionalVariable.notify_one();
}

/* GET */ bool KernelCtrl::getKernelConnected()
{
    return m_isKernelConnected;
}
