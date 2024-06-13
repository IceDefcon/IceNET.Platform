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

const size_t CHAR_DEVICE_SIZE = 32;
const size_t CHAR_CONSOLE_SIZE = 32;

class Kernel_IN : public Compute
{
private:
    int m_file_descriptor;
    std::thread m_Kernel_INThread;
    std::atomic<bool> m_killThread;
    
    /* For char Device Traffic */
    std::vector<char>* m_Kernel_INRx;
    std::vector<char>* m_Kernel_INTx;
    /* Console control buffer */
    std::vector<char>* m_consoleControl;

    bool m_Kernel_INwait;

public:
    Kernel_IN();
    ~Kernel_IN();

    int openDEV();
    int dataTX();
    int dataRX();
    int closeDEV();

    void initThread();
    bool isThreadKilled();

    void Kernel_INThread();

    void setKernel_INTx(std::vector<char>* DataRx);


};
