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

DroneCtrl::DroneCtrl():
m_droneCtrlState(CTRL_INIT)
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate DroneCtrl" << std::endl;
}

DroneCtrl::~DroneCtrl()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy DroneCtrl" << std::endl;
}

void DroneCtrl::droneInit()
{
    std::cout << "[INFO] [DRONE] Drone Initialization" << std::endl;

    /* Get control instances */
    m_instanceServerTCP = this;
    m_instanceCommander = this;
    m_instanceWatchdog = this;
    m_instanceRamDisk = this;

    /* Ram Disk Commander */
    KernelComms::initRamDiskCommander();
    /* TCP Server */
    Network::initServerTCP();
}

void DroneCtrl::droneExit()
{
    std::cout << "[INFO] [DRONE] Drone Exit" << std::endl;
    KernelComms::shutdownRamDiskCommander();
    Network::shutdownServerTCP();
}

bool DroneCtrl::isKilled()
{
    bool retFlag = false;

    retFlag = KernelComms::Watchdog::isWatchdogDead() || KernelComms::Commander::isThreadKilled() || Network::ServerTCP::isThreadKilled();

    return retFlag;
}

void DroneCtrl::sendFpgaConfig()
{
    std::cout << std::endl;
    std::cout << "[INFO] [DRONE] Watchdog ready :: Load FPGA Config to DMA Engine" << std::endl;
    m_instanceRamDisk->AssembleData();
    m_instanceRamDisk->dataTX();
    std::cout << "[INFO] [DRONE] Watchdog ready :: Activate DMA Engine" << std::endl;
    m_instanceCommander->dataTX();
}

void DroneCtrl::droneCtrlMain()
{
    switch(m_droneCtrlState)
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
                std::cout << "[INFO] [DRONE] CTRL_INIT" << std::endl;
                m_droneCtrlState = CTRL_CONFIG;
            }
            break;

        case CTRL_CONFIG:
            std::cout << "[INFO] [DRONE] CTRL_CONFIG" << std::endl;
            sendFpgaConfig();
            m_droneCtrlState = CTRL_IDLE;
            break;

        case CTRL_IDLE:
            m_droneCtrlState = CTRL_INPUT;
            break;

        case CTRL_INPUT:
            // m_commanderState = m_instanceServerTCP->getCommanderState();
            // m_instanceCommander->setCommanderState(m_commanderState);
            m_droneCtrlState = CTRL_OUTPUT;
            break;

        case CTRL_OUTPUT:
            // m_commanderState = m_instanceServerTCP->getCommanderState();
            // m_instanceCommander->setCommanderState(m_commanderState);
            m_droneCtrlState = CTRL_IDLE;
            break;

        default:
            std::cout << "[INFO] [DRONE] Main State" << std::endl;
    }
}
