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

#include "Input.h"
#include "Output.h"
#include "Watchdog.h"
#include "Commander.h"
#include "RamConfig.h"
#include "Types.h"

class KernelComms :
    public Input,
    public Output,
    public Watchdog,
    public Commander,
    public RamConfig
{
    private:

        Commander* m_instanceCommander;
        RamConfig* m_instanceRamConfig;

    public:
        KernelComms();
        ~KernelComms();

        void configInstances();
        void initRamDiskCommander();
};
