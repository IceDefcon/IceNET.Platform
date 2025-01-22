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

#include "Types.h"

class Input
{
private:
    int m_file_descriptor;
    std::thread m_threadInput;
    std::atomic<bool> m_threadKill;
    Input_stateType m_currentState;
    
    /* For char Device Traffic */
    std::vector<char>* m_Rx_Input;
    std::vector<char>* m_Tx_Input;
    int m_Rx_bytesReceived;
    int m_Tx_bytesReceived;

    bool m_waitInput;

public:
    Input();
    ~Input();

    int openDEV();
    int dataTX();
    int dataRX();
    int closeDEV();

    void initThread();
    bool isThreadKilled();

    void threadInput();

    void setTx_Input(std::vector<char>* DataRx, int byteReceived);

    void setInputState(Input_stateType newState);
};
