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

DroneCtrl::DroneCtrl()
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
    KernelComms::configInstances();
    KernelComms::initRamDiskCommander();
}

void DroneCtrl::droneExit()
{
    std::cout << "[INFO] [DRONE] Drone Exit" << std::endl;
    Watchdog::closeDEV();
    Commander::closeDEV();
}

bool DroneCtrl::isKilled()
{
    bool retFlag = false;
    retFlag = KernelComms::Watchdog::isThreadKilled() ? true : false;

    return retFlag;
}


