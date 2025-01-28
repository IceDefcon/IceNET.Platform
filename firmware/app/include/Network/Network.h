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

#include "ServerTCP.h"
#include "Types.h"

class Network :
    public ServerTCP
{
    private:

    public:
        Network();
        ~Network();

        void initServerTCP();
        void shutdownServerTCP();
};
