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

#include "Console.h"
#include "Types.h"

class KernelInput : public Console
{
private:
    int m_file_descriptor;
    std::thread m_threadKernelInput;
    std::atomic<bool> m_threadKill;
    KernelInput_stateType m_currentState;
    
    /* For char Device Traffic */
    std::vector<char>* m_Rx_KernelInput;
    std::vector<char>* m_Tx_KernelInput;
    int m_Rx_bytesReceived;
    int m_Tx_bytesReceived;

    bool m_waitKernelInput;

public:
    KernelInput();
    ~KernelInput();

    int openDEV();
    int dataTX();
    int dataRX();
    int closeDEV();

    void initThread();
    bool isThreadKilled();

    void threadKernelInput();

    void setTx_KernelInput(std::vector<char>* DataRx, int byteReceived);

    void setKernelInputState(KernelInput_stateType newState);
};
