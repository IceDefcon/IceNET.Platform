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
    m_ctrlStatePrev(CTRL_INIT),
    m_Rx_DroneCtrlVector(std::make_shared<std::vector<char>>(IO_TRANSFER_SIZE)),
    m_Tx_DroneCtrlVector(std::make_shared<std::vector<char>>(IO_TRANSFER_SIZE)),
    m_IO_DroneCtrlState(std::make_shared<ioStateType>(IO_IDLE))
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate DroneCtrl" << std::endl;
}

DroneCtrl::~DroneCtrl()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy DroneCtrl" << std::endl;
}

void DroneCtrl::droneInit()
{
    std::cout << "[INIT] [ D ] Drone Initialization" << std::endl;

    /* Get control instances */
    m_instanceServerTCP = this;
    m_instanceCommander = this;
    m_instanceWatchdog = this;
    m_instanceRamDisk = this;
    /* Align shared pointers for Kernel<->ServerTCP communication and StateMachines */
    m_instanceServerTCP->setTransferPointers(m_Rx_DroneCtrlVector, m_Tx_DroneCtrlVector, m_IO_DroneCtrlState);
    m_instanceCommander->setTransferPointers(m_Rx_DroneCtrlVector, m_Tx_DroneCtrlVector, m_IO_DroneCtrlState);
    /* Launch Ram Disk Commander and TCP Server */
    KernelComms::initRamDiskCommander();
    Network::initServerTCP();
}

void DroneCtrl::droneExit()
{
    std::cout << "[INFO] [ D ] Drone Exit" << std::endl;
    KernelComms::shutdownRamDiskCommander();
    Network::shutdownServerTCP();
}

bool DroneCtrl::isKilled()
{
    bool retFlag = false;

    retFlag = KernelComms::Watchdog::isWatchdogDead() || KernelComms::Commander::isThreadKilled() || Network::ServerTCP::isThreadKilled();

    return retFlag;
}

std::string DroneCtrl::getCtrlStateString(ctrlType state)
{
    static const std::array<std::string, IO_AMOUNT> ctrlStateStrings =
    {
        "CTRL_INIT",
        "CTRL_CONFIG",
        "CTRL_RECONFIG_DMA",
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
             * Configure chips connectd to FPGA
             * Either I2C or SPI buses
             * Trough the DMA
             */
            if(true == KernelComms::Watchdog::getFpgaConfigReady())
            {
                m_ctrlState = CTRL_CONFIG;
            }
            break;

        case CTRL_CONFIG:
            sendFpgaConfig();
            m_ctrlState = CTRL_RECONFIG_DMA;
            break;

        case CTRL_RECONFIG_DMA:
            /**
             * TODO
             *
             * This cannot be delayed by 1000ms
             *
             * Proposed solution is feedback from Kernel Commander
             * To compare with pre-defined value
             *
             * But for now 1000ms delay to reconfig DMA into singe mode
             *
             */
            std::cout << "[INFO] [ D ] CTRL_RECONFIG_DMA :: Delay Start" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            std::cout << "[INFO] [ D ] CTRL_RECONFIG_DMA :: Delay Stop" << std::endl;
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
