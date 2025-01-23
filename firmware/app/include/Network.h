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

#include "NetworkTraffic.h"
#include "ServerTCP.h"
#include "Types.h"

class Network :
    public ServerTCP,
    public NetworkTraffic
{
    private:

    public:
        Network();
        ~Network();

};
