/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <chrono> // delay
#include <thread> // delay

// #include "NetworkTraffic.h"
// #include "Output.h"
// #include "Input.h"
// #include "RamConfig.h"
// #include "ServerTCP.h"
// #include "Commander.h"
// #include "Watchdog.h"

#include "DroneCtrl.h"

int main() 
{
#if 0
    /* Smart pointers for auto Heap allocation and dealocation */
    auto instanceNetworkTraffic = std::make_shared<NetworkTraffic>();
    auto instanceWatchdog = std::make_shared<Watchdog>();
    auto instanceInput = std::make_shared<Input>();
    auto instanceRamConfig = std::make_shared<RamConfig>();
    auto instanceOutput = std::make_shared<Output>();
    auto instanceServerTCP = std::make_shared<ServerTCP>();
    auto instanceCommander = std::make_shared<Commander>();

    /* Set Instances */
    instanceOutput->setInstance_NetworkTraffic(instanceNetworkTraffic);
    instanceServerTCP->setInstance_NetworkTraffic(instanceNetworkTraffic);
    instanceServerTCP->setInstance_RamConfig(instanceRamConfig);
    instanceNetworkTraffic->setInstance_Input(instanceInput);
    instanceWatchdog->setInstance_RamConfig(instanceRamConfig);
    instanceWatchdog->setInstance_Commander(instanceCommander);

    /* Initialize Interfaces */
    instanceCommander->openDEV();
    instanceWatchdog->openDEV();
    instanceInput->openDEV();
    instanceOutput->openDEV();
    instanceServerTCP->openDEV();
    instanceNetworkTraffic->openDEV();

    while (true) /* Terminate Kernel comms and Clean Memory */
    {
        if (instanceOutput->isThreadKilled() || instanceWatchdog->isThreadKilled())
        {
            instanceWatchdog->closeDEV();
            instanceServerTCP->closeDEV();
            instanceOutput->closeDEV();
            instanceInput->closeDEV();
            instanceNetworkTraffic->closeDEV();
            instanceCommander->closeDEV();
            break;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
#endif

    auto instanceDroneCtrl = std::make_shared<DroneCtrl>();
    instanceDroneCtrl->Commander::openDEV();
    instanceDroneCtrl->Watchdog::openDEV();

    while (true) /* Terminate Kernel comms and Clean Memory */
    {
        if (instanceDroneCtrl->Output::isThreadKilled() || instanceDroneCtrl->Watchdog::isThreadKilled())
        {
            instanceDroneCtrl->Watchdog::closeDEV();
            instanceDroneCtrl->Commander::closeDEV();
            break;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    /* shared_ptr in use :: No need for deallocation */
    return 0;
}
