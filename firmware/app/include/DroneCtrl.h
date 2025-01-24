/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <vector>

#include "KernelComms.h"
#include "Network.h"
#include "Types.h"

class DroneCtrl :
    public Network,
    public KernelComms
{
    private:

    public:
        DroneCtrl();
        ~DroneCtrl();

        void droneInit();
        void droneExit();
        bool isKilled();
};
