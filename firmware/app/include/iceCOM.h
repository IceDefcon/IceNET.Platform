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
#include <mutex>
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
    std::vector<char>* m_iceCOMRx;
    std::vector<char>* m_iceCOMTx;
    /* Console control buffer */
    std::vector<char>* m_consoleControl;

    std::mutex m_iceCOMmutex;

public:
    iceCOM();
    ~iceCOM();

    int openDEV();
    int dataTX();
    int dataRX();
    int closeDEV();

    void initThread();
    bool isThreadKilled();

    void iceCOMThread();

    void setIceCOMTx(std::vector<char>* DataRx);


};
