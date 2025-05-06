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
#include "Types.h"

class DroneCtrl : public KernelComms
{
    private:
        droneCtrlStateType m_ctrlState;
        droneCtrlStateType m_ctrlStatePrev;

        static constexpr uint32_t FPGA_DELAY = 3000;

    public:
        DroneCtrl();
        ~DroneCtrl();

        void initKernelComms();
        void shutdownKernelComms();
        bool isKilled();

        std::string getCtrlStateString(droneCtrlStateType state);

        void sendFpgaConfigToRamDisk();
        void setDroneCtrlState(droneCtrlStateType state);
        void droneCtrlMain();
};
