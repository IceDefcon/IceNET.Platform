/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#include <chrono> // delay
#include <thread> // delay
#include <iostream>
#include <cstring>
#include <iomanip> // Include the <iomanip> header for setw and setfill
#include <fcntl.h> // For open, O_RDWR, etc.
#include <unistd.h>// For close, read, write, etc.

#include "Network.h"
#include "Types.h"

Network::Network()
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate Network" << std::endl;
}

Network::~Network()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy Network" << std::endl;
}

void Network::initServerTCP()
{
    std::cout << "[INFO] [NET] " << "Initialize TCP Server" << std::endl;
    ServerTCP::initThread();
}

void Network::shutdownServerTCP()
{
    std::cout << "[INFO] [NET] " << "Shutdown TCP Server" << std::endl;
    Network::ServerTCP::shutdownThread();
}
