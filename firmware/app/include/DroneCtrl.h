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
#include "ServerTCP.h"
#include "Types.h"

class DroneCtrl :
    public ServerTCP,
    public KernelComms
{
    private:

    public:
        DroneCtrl();
        ~DroneCtrl();

};
