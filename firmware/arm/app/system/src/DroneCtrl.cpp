/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#include <chrono> // delay
#include <thread> // delay
#include <iostream>
#include <cstring>
#include <iomanip> // Include the <iomanip> header for setw and setfill
#include <fcntl.h> // For open, O_RDWR, etc.
#include <unistd.h>// For close, read, write, etc.

#include "DroneCtrl.h"
#include "Types.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DroneCtrl::DroneCtrl() :
    m_ctrlState(DRONE_CTRL_IDLE),
    m_ctrlStatePrev(DRONE_CTRL_IDLE),
    m_isKernelConnected(false)
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate DroneCtrl" << std::endl;

    initThread();
}

DroneCtrl::~DroneCtrl()
{
    shutdownKernelComms();
    shutdownThread();

    m_isKernelConnected = false;
    
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy DroneCtrl" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* THREAD */ void DroneCtrl::initThread()
{
    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadHandler.joinable())
    {
        std::cout << "[INFO] [ D ] DroneCtrlThread is already running" << std::endl;
        return;
    }

    m_threadKill = false;
    m_threadHandler = std::thread(&DroneCtrl::DroneCtrlThread, this);
    std::cout << "[INFO] [ D ] DroneCtrlThread Initialized" << std::endl;
}

/* THREAD */ void DroneCtrl::shutdownThread()
{
    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadKill)
    {
        std::cout << "[INFO] [ D ] DroneCtrlThread is already marked for shutdown" << std::endl;
        return;
    }

    m_threadKill = true;
    triggerDroneControlEvent();

    if (m_threadHandler.joinable())
    {
        m_threadHandler.join();
    }

    std::cout << "[INFO] [ D ] DroneCtrlThread Terminated" << std::endl;
}

/* THREAD */ std::string DroneCtrl::getThreadStateMachineString(droneCtrlStateType state)
{
    static const std::array<std::string, DRONE_CTRL_AMOUNT> ctrlStateStrings =
    {
        "DRONE_CTRL_IDLE",
        "DRONE_CTRL_SKIP",
        "DRONE_CTRL_INIT",
        "DRONE_CTRL_CONFIG",
        "DRONE_CTRL_RAMDISK_PERIPHERALS",
        "DRONE_CTRL_RAMDISK_ACTIVATE_DMA",
        "DRONE_CTRL_DMA_SINGLE",
        "DRONE_CTRL_MAIN",
    };

    if (state >= 0 && state < DRONE_CTRL_AMOUNT)
    {
        return ctrlStateStrings[state];
    }
    else
    {
        return "UNKNOWN_STATE";
    }
}

/* THREAD */ void DroneCtrl::setDroneCtrlState(droneCtrlStateType state)
{
    std::unique_lock<std::mutex> lock(m_ctrlMutex);
    m_ctrlState = state;
    triggerDroneControlEvent();
}

/* THREAD */ bool DroneCtrl::setKernelComms()
{
    m_isKernelConnected = (OK == initKernelComms()) ? true : false;
    if(false == m_isKernelConnected)
    {
        std::cout << "[INFO] [ D ] Kernel Connection -> Failure" << std::endl;
        m_ctrlState = DRONE_CTRL_IDLE;

        return false;
    }
    else
    {
        std::cout << "[INFO] [ D ] Kernel Connection -> Success" << std::endl;
    }

    return true;
}

/* THREAD */ void DroneCtrl::DroneCtrlThread()
{
    while (!m_threadKill)
    {
        if(m_ctrlStatePrev != m_ctrlState)
        {
            std::cout << "[INFO] [ D ] State DroneCtrl " << m_ctrlStatePrev << "->" << m_ctrlState << " " << getThreadStateMachineString(m_ctrlState) << std::endl;
            m_ctrlStatePrev = m_ctrlState;
        }

        switch(m_ctrlState)
        {
            case DRONE_CTRL_IDLE:
                waitDroneControlEvent();
                break;

            case DRONE_CTRL_SKIP:
                std::cout << "[INFO] [ D ] Configuration Skipped :: Switch DMA into a Normal Mode" << std::endl;
                if(false == setKernelComms())
                {
                    break;
                }
                else
                {
                    /* ALL GOOD */
                }


                m_ctrlState = DRONE_CTRL_DMA_SINGLE;
                break;

            case DRONE_CTRL_INIT:
                m_isKernelConnected = (OK == initKernelComms()) ? true : false;
                if(false == setKernelComms())
                {
                    break;
                }
                else
                {
                    /* ALL GOOD */
                }

                m_ctrlState = DRONE_CTRL_CONFIG;
                break;

            case DRONE_CTRL_CONFIG:
                /**
                 * Main Config
                 *
                 * Configure IMU's connectd to FPGA
                 * Via I2C or SPI buses
                 * Using SPI/DMA
                 */
                if(true == KernelComms::Watchdog::getFpgaConfigReady())
                {
                    sendCommand(CMD_RAMDISK_CLEAR);
                    m_ctrlState = DRONE_CTRL_RAMDISK_PERIPHERALS;
                }
                break;

            case DRONE_CTRL_RAMDISK_PERIPHERALS:
                sendFpgaConfigToRamDisk();
                m_ctrlState = DRONE_CTRL_RAMDISK_ACTIVATE_DMA;
                break;

            case DRONE_CTRL_RAMDISK_ACTIVATE_DMA:
                std::cout << "[INFO] [ D ] Activating RamDisk Config DMA Engine" << std::endl;
                sendCommand(CMD_RAMDISK_CONFIG);
                /* Wait for Kerenl to send data to FPGA */
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                m_ctrlState = DRONE_CTRL_DMA_SINGLE;
                break;

            case DRONE_CTRL_DMA_SINGLE:
                std::cout << "[INFO] [ D ] Configuration Done :: Switch DMA into a Normal Mode" << std::endl;
                sendCommand(CMD_DMA_NORMAL);
                m_ctrlState = DRONE_CTRL_IDLE;
                break;

            case DRONE_CTRL_MAIN:
                //
                // TODO
                //
                break;

            default:
                std::cout << "[INFO] [ D ] Unknown State" << std::endl;
        }
    }

    std::cout << "[INFO] [ D ] Terminate DroneCtrlThread" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int DroneCtrl::initKernelComms()
{
    int ret = UNKNOWN;

    std::cout << "[INIT] [ D ] Initialize RamDisk Commander" << std::endl;
    ret = KernelComms::initRamDiskCommander();

    return ret;
}

void DroneCtrl::shutdownKernelComms()
{
    std::cout << "[INFO] [ D ] Drone Exit" << std::endl;
    KernelComms::shutdownRamDiskCommander();
}

bool DroneCtrl::isKernelComsDead()
{
    bool ret = false;

    ret = KernelComms::Watchdog::isWatchdogDead() || KernelComms::Commander::isThreadKilled();

    return ret;
}

void DroneCtrl::sendFpgaConfigToRamDisk()
{
    std::cout << "[INFO] [ D ] Peripherals configuration ready :: Loading to FPGA" << std::endl;
    assembleConfig();
    sendConfig();
}

/* EVENT */ void DroneCtrl::waitDroneControlEvent()
{
    std::cout << "[INFO] [ D ] DroneCtrlThread Wait" << std::endl;
    std::unique_lock<std::mutex> lock(m_eventMutex);

    auto predicate = [this]()
    {
        return m_stateChanged;
    };

    m_conditionalVariable.wait(lock, predicate);
    m_stateChanged = false;
}

/* EVENT */ void DroneCtrl::triggerDroneControlEvent()
{
    std::cout << "[INFO] [ D ] DroneCtrlThread Event" << std::endl;
    std::lock_guard<std::mutex> lock(m_eventMutex);

    m_stateChanged = true;
    m_conditionalVariable.notify_one();
}

/* GET */ bool DroneCtrl::getKernelConnected()
{
    return m_isKernelConnected;
}
