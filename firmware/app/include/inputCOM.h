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

class inputCOM : public Compute
{
private:
    int m_file_descriptor;
    std::thread m_inputCOMThread;
    std::atomic<bool> m_killThread;
    
    /* For char Device Traffic */
    std::vector<char>* m_inputCOMRx;
    std::vector<char>* m_inputCOMTx;
    /* Console control buffer */
    std::vector<char>* m_consoleControl;

    bool m_inputCOMwait;

public:
    inputCOM();
    ~inputCOM();

    int openDEV();
    int dataTX();
    int dataRX();
    int closeDEV();

    void initThread();
    bool isThreadKilled();

    void inputCOMThread();

    void setinputCOMTx(std::vector<char>* DataRx);


};
