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

#include "Compute.h"
#include "Types.h"

const size_t CHAR_DEVICE_SIZE = 32;
const size_t CHAR_CONSOLE_SIZE = 32;

class Kernel_IN : public Compute
{
private:
    int m_file_descriptor;
    std::thread m_threadKernel_IN;
    std::atomic<bool> m_threadKill;
    Kernel_IN_stateType m_currentState;
    
    /* For char Device Traffic */
    std::vector<char>* m_Rx_Kernel_IN;
    std::vector<char>* m_Tx_Kernel_IN;
    /* Console control buffer */
    std::vector<char>* m_consoleControl;

    bool m_waitKernel_IN;

public:
    Kernel_IN();
    ~Kernel_IN();

    int openDEV();
    int dataTX();
    int dataRX();
    int closeDEV();

    void initThread();
    bool isThreadKilled();

    void threadKernel_IN();

    void setTx_Kernel_IN(std::vector<char>* DataRx);

    void setKernel_INState(Kernel_IN_stateType newState);
};
