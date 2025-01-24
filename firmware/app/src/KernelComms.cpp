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

#include "KernelComms.h"
#include "Network.h"
#include "Types.h"

KernelComms::KernelComms()
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate KernelComms" << std::endl;
}

KernelComms::~KernelComms()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy KernelComms" << std::endl;
}

void KernelComms::configInstances()
{
    std::cout << "[INFO] [KIO] " << "Configure instances of comms classes" << std::endl;

    m_instanceCommander = Commander::getInstance();
    m_instanceRamConfig = RamConfig::getInstance();
    Watchdog::setInstance_Commander(m_instanceCommander);
    Watchdog::setInstance_RamConfig(m_instanceRamConfig);
}

void KernelComms::initRamDiskCommander()
{
    Commander::openDEV();
    Watchdog::openDEV();
}
