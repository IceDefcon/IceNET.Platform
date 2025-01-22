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
#include "Types.h"

class KernelComms :
    public Input,
    public Output,
    public Watchdog,
    public Commander,
    public RamConfig
{
    private:

    public:
        KernelComms();
        ~KernelComms();

};
