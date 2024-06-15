/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <chrono> // delay
#include <thread> // delay

#include "Kernel_IN.h"
#include "Kernel_OUT.h"
#include "ServerTCP.h"
#include "NetworkTraffic.h"

int main() 
{
    /* Smart pointers for auto Heap allocation and dealocation */
    auto instanceNetworkTraffic = std::make_shared<NetworkTraffic>();

    auto instanceKernel_IN = std::make_shared<Kernel_IN>();
    auto instanceKernel_OUT = std::make_shared<Kernel_OUT>();
    auto instanceServerTCP = std::make_shared<ServerTCP>();

    /* Set Instances */
    instanceKernel_OUT->setInstance_NetworkTraffic(instanceNetworkTraffic);
    instanceServerTCP->setInstance_NetworkTraffic(instanceNetworkTraffic);
    instanceNetworkTraffic->setInstance_Kernel_IN(instanceKernel_IN);

    /* Initialize Interfaces */
    instanceKernel_IN->openDEV();
    instanceKernel_OUT->openDEV();
    instanceServerTCP->openDEV();
    instanceNetworkTraffic->openDEV();

    while (true) /* Terminate Kernel comms and Clean Memory */
    {
        if (instanceKernel_OUT->isThreadKilled()) 
        {
            instanceServerTCP->closeDEV();
            instanceKernel_OUT->closeDEV();
            instanceKernel_IN->closeDEV();
            instanceNetworkTraffic->closeDEV();
            break;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    /* shared_ptr in use :: No need for deallocation */
    return 0;
}
