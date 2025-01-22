/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <chrono> // delay
#include <thread> // delay
#include "DroneCtrl.h"

int main()
{
    /**
     * Smart pointer for auto Heap
     * allocation and dealocation
     */
    auto instanceDroneCtrl = std::make_shared<DroneCtrl>();

    instanceDroneCtrl->KernelComms::Commander::openDEV();
    instanceDroneCtrl->KernelComms::Watchdog::openDEV();
    instanceDroneCtrl->KernelComms::Input::openDEV();
    instanceDroneCtrl->KernelComms::Output::openDEV();

    while (true) /* Terminate Kernel comms and Clean Memory */
    {
        if (instanceDroneCtrl->Output::isThreadKilled() || instanceDroneCtrl->Watchdog::isThreadKilled())
        {
            instanceDroneCtrl->KernelComms::Watchdog::closeDEV();
            instanceDroneCtrl->KernelComms::Output::closeDEV();
            instanceDroneCtrl->KernelComms::Input::closeDEV();
            instanceDroneCtrl->KernelComms::Commander::closeDEV();
            break;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    /* shared_ptr in use :: No need for deallocation */
    return 0;
}

#if 0 /* Old Costruction */

    /* Initialize Interfaces */
    instanceCommander->openDEV();
    instanceWatchdog->openDEV();
    instanceInput->openDEV();
    instanceOutput->openDEV();
    instanceServerTCP->openDEV();
    instanceNetworkTraffic->openDEV();

    /* Terminate Kernel comms and Clean Memory */
    instanceWatchdog->closeDEV();
    instanceServerTCP->closeDEV();
    instanceOutput->closeDEV();
    instanceInput->closeDEV();
    instanceNetworkTraffic->closeDEV();
    instanceCommander->closeDEV();

#endif
