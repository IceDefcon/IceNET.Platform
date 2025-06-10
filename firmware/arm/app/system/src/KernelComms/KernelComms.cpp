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

#include "KernelComms.h"
#include "Types.h"

KernelComms::KernelComms()
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate KernelComms" << std::endl;
}

KernelComms::~KernelComms()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy KernelComms" << std::endl;
}

int KernelComms::initRamDiskCommander()
{
    int ret;

    ret = Commander::openDEV();
    if(OK != ret)
    {
        std::cout << "[ERNO] [ K ] Commander Problem -> " << ret << std::endl;
        return ret;
    }

    Watchdog::initBuffers();

    ret = Watchdog::openDEV();
    if(OK != ret)
    {
        std::cout << "[ERNO] [ K ] Watchdog Problem -> " << ret << std::endl;
        return ret;
    }

    return ret;
}

void KernelComms::shutdownRamDiskCommander()
{
    Commander::shutdownThread();
    Watchdog::shutdownThread();
}
