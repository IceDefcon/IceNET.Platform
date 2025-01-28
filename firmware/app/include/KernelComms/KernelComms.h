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

#include "Watchdog.h"
#include "Commander.h"
#include "RamDisk.h"
#include "Types.h"

class KernelComms :
    public Watchdog,
    public Commander,
    public RamDisk
{
    private:

        Commander* m_instanceCommander;
        RamDisk* m_instanceRamDisk;

    public:
        KernelComms();
        ~KernelComms();

        void configInstances();

        void initRamDiskCommander();
        void shutdownRamDiskCommander();
};
