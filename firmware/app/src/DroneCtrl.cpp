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
    m_ioState(IO_IDLE),
    m_ctrlState(CTRL_INIT),
    m_ctrlStatePrev(CTRL_INIT),
    m_Rx_DroneCtrlVector(std::make_shared<std::vector<char>>(IO_TRAMSFER_SIZE)),
    m_Tx_DroneCtrlVector(std::make_shared<std::vector<char>>(IO_TRAMSFER_SIZE)),
    m_Rx_DroneCtrl(new std::vector<char>(IO_TRAMSFER_SIZE)),
    m_Tx_DroneCtrl(new std::vector<char>(IO_TRAMSFER_SIZE)),
    m_Rx_bytesReceived(0),
    m_Tx_bytesReceived(0)
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

    m_instanceServerTCP->setTransferPointer(m_Rx_DroneCtrlVector, m_Tx_DroneCtrlVector);
    m_instanceCommander->setTransferPointer(m_Rx_DroneCtrlVector, m_Tx_DroneCtrlVector);

    std::cout << "[INIT] [ D ] Initialise DroneCtrl Buffers" << std::endl;
    std::fill(m_Rx_DroneCtrl->begin(), m_Rx_DroneCtrl->end(), 0);
    std::fill(m_Tx_DroneCtrl->begin(), m_Tx_DroneCtrl->end(), 0);

    /* Ram Disk Commander */
    KernelComms::initRamDiskCommander();
    /* TCP Server */
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
        "CTRL_IDLE",
        "CTRL_COMMANDER",
        "CTRL_SERVER",
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
    m_instanceRamDisk->AssembleData();
    m_instanceRamDisk->dataTX();
    std::cout << "[INFO] [ D ] Watchdog ready :: Activate DMA Engine" << std::endl;
    m_instanceCommander->dataTX();
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
            m_ctrlState = CTRL_IDLE;
            break;

        case CTRL_IDLE:
            if(m_ioState != m_instanceServerTCP->getIO_State())
            {
#if 0
                std::cout << "[INFO] [ D ] State Commander " << m_ioState << "->" << m_instanceServerTCP->getIO_State() << " " << getIoStateString(m_instanceServerTCP->getIO_State()) << std::endl;
#endif
                m_ioState = m_instanceServerTCP->getIO_State();
                m_ctrlState = CTRL_COMMANDER;
            }
            else if(m_ioState != m_instanceCommander->getIO_State())
            {
#if 0
                std::cout << "[INFO] [ D ] State ServerTCP " << m_ioState << "->" << m_instanceCommander->getIO_State() << " " << getIoStateString(m_instanceCommander->getIO_State()) << std::endl;
#endif
                m_ioState = m_instanceCommander->getIO_State();
                m_ctrlState = CTRL_SERVER;
            }
            break;

        case CTRL_COMMANDER: /* Data to Kernel */
            m_Rx_bytesReceived = m_instanceServerTCP->getRx_ServerTCP(*m_Rx_DroneCtrl);
            m_instanceCommander->setTx_Commander(*m_Rx_DroneCtrl, m_Rx_bytesReceived);
            m_instanceCommander->setIO_State(m_ioState);
            m_ctrlState = CTRL_IDLE;
            break;

        case CTRL_SERVER: /* Data From Kernel */
            m_Tx_bytesReceived = m_instanceCommander->getRx_Commander(*m_Tx_DroneCtrl);
            m_instanceServerTCP->setTx_ServerTCP(*m_Tx_DroneCtrl, m_Tx_bytesReceived);
            m_instanceServerTCP->setIO_State(m_ioState);
            m_ctrlState = CTRL_IDLE;
            break;

        default:
            std::cout << "[INFO] [ D ] Main State" << std::endl;
    }
}
