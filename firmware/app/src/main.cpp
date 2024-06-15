/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <chrono> // delay
#include <thread> // delay

#include "KernelInput.h"
#include "KernelOutput.h"
#include "ServerTCP.h"
#include "NetworkTraffic.h"

int main() 
{
    /* Smart pointers for auto Heap allocation and dealocation */
    auto instanceNetworkTraffic = std::make_shared<NetworkTraffic>();

    auto instanceKernelInput = std::make_shared<KernelInput>();
    auto instanceKernelOutput = std::make_shared<KernelOutput>();
    auto instanceServerTCP = std::make_shared<ServerTCP>();

    /* Set Instances */
    instanceKernelOutput->setInstance_NetworkTraffic(instanceNetworkTraffic);
    instanceServerTCP->setInstance_NetworkTraffic(instanceNetworkTraffic);
    instanceNetworkTraffic->setInstance_KernelInput(instanceKernelInput);

    /* Initialize Interfaces */
    instanceKernelInput->openDEV();
    instanceKernelOutput->openDEV();
    instanceServerTCP->openDEV();
    instanceNetworkTraffic->openDEV();

    while (true) /* Terminate Kernel comms and Clean Memory */
    {
        if (instanceKernelOutput->isThreadKilled()) 
        {
            instanceServerTCP->closeDEV();
            instanceKernelOutput->closeDEV();
            instanceKernelInput->closeDEV();
            instanceNetworkTraffic->closeDEV();
            break;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    /* shared_ptr in use :: No need for deallocation */
    return 0;
}
