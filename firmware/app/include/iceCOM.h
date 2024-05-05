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

#include "debug.h"
#include "core.h"


const size_t CHAR_DEVICE_SIZE = 32;
const size_t CONSOLE_TERMINAL_SIZE = 32;

class iceCOM : public Core
{

private:
    int m_file_descriptor;
    std::thread m_iceThread;

    /*!
     * 
     * Atomic in case if something 
     * decide to interrupt kill 
     * flag processing
     * 
     */
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

    int device_open(const char* device) override;
    int device_read() override;
    int device_write() override;
    int device_close() override;

    bool terminate();
};