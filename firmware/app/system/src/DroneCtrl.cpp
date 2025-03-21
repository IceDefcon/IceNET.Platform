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

void DroneCtrl::initPointers()
{
    std::cout << "[INIT] [ D ] Setup the pointers" << std::endl;
    /* Get control instances */
    m_instanceCommander = this;
    m_instanceWatchdog = this;
    m_instanceRamDisk = this;
}

Commander* DroneCtrl::getCommanderInstance()
{
    return m_instanceCommander;
}

void DroneCtrl::initKernelComms()
{
    std::cout << "[INIT] [ D ] Initialize RamDisk Commander" << std::endl;
    KernelComms::initRamDiskCommander();
}

void DroneCtrl::shutdownKernelComms()
{
    std::cout << "[INFO] [ D ] Drone Exit" << std::endl;
    KernelComms::shutdownRamDiskCommander();
}

bool DroneCtrl::isKilled()
{
    bool ret = false;

    ret = KernelComms::Watchdog::isWatchdogDead() || KernelComms::Commander::isThreadKilled();

    return ret;
}

std::string DroneCtrl::getCtrlStateString(ctrlType state)
{
    static const std::array<std::string, CTRL_AMOUNT> ctrlStateStrings =
    {
        "CTRL_INIT",
        "CTRL_DMA_LONG",
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

void DroneCtrl::sendFpgaConfig()
{
    std::cout << "[INFO] [ D ] Watchdog ready :: Load FPGA Config to DMA Engine" << std::endl;
    m_instanceRamDisk->assembleConfig();
    m_instanceRamDisk->sendConfig();
    std::cout << "[INFO] [ D ] Watchdog ready :: Activate DMA Engine" << std::endl;
    m_instanceCommander->sendCommand(CMD_FPGA_CONFIG);
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
                m_ctrlState = CTRL_DMA_LONG;
            }
            break;

        case CTRL_DMA_LONG:
            sendFpgaConfig();
            m_ctrlState = CTRL_DMA_SINGLE;
            break;

        case CTRL_DMA_SINGLE:
            /**
             * TODO
             *
             * This cannot be delayed by 3000ms
             *
             * Proposed solution is feedback from Kernel Commander
             * To compare with pre-defined value
             *
             * But for now 3000ms delay to reconfig DMA into singe mode
             *
             */
            std::cout << "[INFO] [ D ] Waiting " << static_cast<uint32_t>(FPGA_DELAY) << "ms for the FPGA to configure Peripherals..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            std::cout << "[INFO] [ D ] Configuration Done :: Switch DMA into a Single Mode" << std::endl;
            m_instanceCommander->sendCommand(CMD_DMA_RECONFIG);
            m_ctrlState = CTRL_MAIN;
            break;

        case CTRL_MAIN:
            /* TODO :: Main Function */
            break;

        default:
            std::cout << "[INFO] [ D ] Main State" << std::endl;
    }
}

