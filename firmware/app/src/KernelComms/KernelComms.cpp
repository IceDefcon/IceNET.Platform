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
    m_instanceRamDisk = RamDisk::getInstance();
    Watchdog::setCommanderInstance(m_instanceCommander);
    Watchdog::setRamDiskInstance(m_instanceRamDisk);
}

void KernelComms::initRamDiskCommander()
{
    std::cout << "[INFO] [KIO] " << "Open (C)Commander and (B)RamDisk devices" << std::endl;

    Commander::initBuffers();
    Watchdog::initBuffers();

    Commander::openDEV();
    Watchdog::openDEV();
}

void KernelComms::shutdownRamDiskCommander()
{
    std::cout << "[INFO] [KIO] " << "Close (C)Commander and (B)RamDisk devices" << std::endl;
    Watchdog::closeDEV();
    Commander::closeDEV();
}
