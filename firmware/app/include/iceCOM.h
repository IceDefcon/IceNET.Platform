/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include <semaphore.h>

#include "core.h"
#include "console.h"

const size_t CHAR_DEVICE_SIZE = 32;
const size_t CHAR_CONSOLE_SIZE = 32;

class iceCOM : public Core
{

    private:
        int m_file_descriptor;
        std::thread m_iceCOMThread;
        std::atomic<bool> m_killThread;
        
        /* For char Device Traffic */
        std::vector<char> charDeviceRx;
        std::vector<char> charDeviceTx;
        /* Console control buffer */
        std::vector<char> consoleControl;

    public:

        iceCOM();
        ~iceCOM();

        void initThread();
        void iceCOMThread();

        int openCOM() override;
        int dataTX() override;
        int dataRX() override;
        int closeCOM() override;

        uint8_t computeDeviceAddress(const char* in);
        uint8_t computeRegisterAddress(const char* in);
        uint8_t computeRegisterControl(const char* in);
        uint8_t computeRegisterData(const char* in);

        bool terminate() override;
};