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

DroneCtrl::DroneCtrl() :
    m_ctrlState(CTRL_INIT),
    m_ctrlStatePrev(CTRL_INIT)
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate DroneCtrl" << std::endl;
}

DroneCtrl::~DroneCtrl()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy DroneCtrl" << std::endl;
}

void DroneCtrl::initKernelComms()
{
    std::cout << "[INIT] [ D ] Initialize RamDisk Commander" << std::endl;
    KernelComms::initRamDiskCommander();
}

void DroneCtrl::shutdownKernelComms(bool isKernelConnected)
{
    std::cout << "[INFO] [ D ] Drone Exit" << std::endl;
    KernelComms::shutdownRamDiskCommander(isKernelConnected);
}

bool DroneCtrl::isKilled()
{
    bool ret = false;

    ret = KernelComms::Watchdog::isWatchdogDead() || KernelComms::Commander::isThreadKilled();

    return ret;
}

std::string DroneCtrl::getCtrlStateString(droneCtrlStateType state)
{
    static const std::array<std::string, CTRL_AMOUNT> ctrlStateStrings =
    {
        "CTRL_INIT",
        "CTRL_RAMDISK_PERIPHERALS",
        "CTRL_RAMDISK_ACTIVATE_DMA",
        "CTRL_DMA_SINGLE",
        "CTRL_MAIN",
    };

    if (state >= 0 && state < CTRL_AMOUNT)
    {
        return ctrlStateStrings[state];
    }
    else
    {
        return "UNKNOWN_STATE";
    }
}

void DroneCtrl::sendFpgaConfigToRamDisk()
{
    std::cout << "[INFO] [ D ] Peripherals configuration ready :: Loading to FPGA" << std::endl;
    assembleConfig();
    sendConfig();
}

void DroneCtrl::setDroneCtrlState(droneCtrlStateType state)
{
    while(CTRL_MAIN != m_ctrlState) /* Wait until we are in CTRL_MAIN */
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    m_ctrlState = state;
}

void DroneCtrl::droneCtrlMain()
{
#if 1 /* Just another debug */
    if(m_ctrlStatePrev != m_ctrlState)
    {
        std::cout << "[INFO] [ D ] State DroneCtrl " << m_ctrlStatePrev << "->" << m_ctrlState << " " << getCtrlStateString(m_ctrlState) << std::endl;
        m_ctrlStatePrev = m_ctrlState;
    }
#endif
    switch(m_ctrlState)
    {
        case CTRL_INIT:
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
                m_ctrlState = CTRL_RAMDISK_PERIPHERALS;
            }
            break;

        case CTRL_RAMDISK_PERIPHERALS:
            sendFpgaConfigToRamDisk();
            m_ctrlState = CTRL_RAMDISK_ACTIVATE_DMA;
            break;

        case CTRL_RAMDISK_ACTIVATE_DMA:
            std::cout << "[INFO] [ D ] Activating RamDisk Config DMA Engine" << std::endl;
            sendCommand(CMD_RAMDISK_CONFIG);
            /* Wait for Kerenl to send data to FPGA */
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            m_ctrlState = CTRL_DMA_SINGLE;
            break;

        case CTRL_DMA_SINGLE:
            std::cout << "[INFO] [ D ] Configuration Done :: Switch DMA into a Normal Mode" << std::endl;
            sendCommand(CMD_DMA_NORMAL);
            m_ctrlState = CTRL_MAIN;
            break;

        case CTRL_MAIN:
            //
            // TODO
            //
            break;

        default:
            std::cout << "[INFO] [ D ] Unknown State" << std::endl;
    }

    /* Reduce consumption of CPU resources */
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
