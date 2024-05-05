/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#pragma once

#include <thread>
#include <atomic>
#include <semaphore.h>

#include "debug.h"
#include "core.h"

#include <vector>

const size_t BUFFER_LENGTH = 32;

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

    std::vector<char> console_RX;
    std::vector<char> console_TX;

public:

    iceCOM() {}
    ~iceCOM() {}

    void initThread();
    void iceCOMThread();

    int device_open(const char* device) override;
    int device_read() override;
    int device_write() override;
    int device_close() override;

    bool terminate();
};