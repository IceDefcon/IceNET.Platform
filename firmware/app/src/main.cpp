/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#include <chrono> // delay
#include <thread> // delay

#include "DroneCtrl.h"

int main()
{
    /**
     * Smart pointer for auto Heap
     * allocation and deallocation
     */
    auto instanceDroneCtrl = std::make_unique<DroneCtrl>();
    instanceDroneCtrl->droneInit();

    while (true)
    {
        instanceDroneCtrl->droneCtrlMain();

        if (true == instanceDroneCtrl->isKilled()) /* Terminate Drone Application threads and Clean Memory */
        {
            std::cout << "[EXIT] [TERMINATE] Shutdown Application" << std::endl;
            instanceDroneCtrl->droneExit();
            break;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    /* unique_ptr in use :: No need for deallocation */
    return 0;
}
