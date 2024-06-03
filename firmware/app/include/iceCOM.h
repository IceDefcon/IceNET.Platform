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

#include "compute.h"

const size_t CHAR_DEVICE_SIZE = 32;
const size_t CHAR_CONSOLE_SIZE = 32;

class iceCOM : public Compute
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

        int openCOM();
        int dataTX();
        int dataRX();
        int closeCOM();

        void initThread();
        bool isThreadKilled();

        void iceCOMThread();
};