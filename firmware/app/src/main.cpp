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
     * allocation and dealocation
     */
    auto instanceDroneCtrl = std::make_shared<DroneCtrl>();
    instanceDroneCtrl->droneInit();

#if 1 /* Test */

    while (true) /* Terminate Drone Application threads and Clean Memory */
    {
        if (true == instanceDroneCtrl->isKilled())
        {
            std::cout << "[EXIT] [TERMINATE] Shutdown Application" << std::endl;
            instanceDroneCtrl->droneExit();
            break;
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
#endif

    /* shared_ptr in use :: No need for deallocation */
    return 0;
}

