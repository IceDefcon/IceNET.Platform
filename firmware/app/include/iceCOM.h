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
        std::vector<char> m_charDeviceRx;
        std::vector<char> m_charDeviceTx;
        /* Console control buffer */
        std::vector<char> m_consoleControl;

        /* Data ready :: For SM */
        bool m_charRxReady;
        bool m_charTxReady;

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

        std::vector<char>* getCharDeviceRx();
        void setCharDeviceTx(std::vector<char>* charVector);

        bool getCharRxReady();
        void setCharRxReady(bool flag);
        bool getCharTxReady();
        void setCharTxReady(bool flag);

};