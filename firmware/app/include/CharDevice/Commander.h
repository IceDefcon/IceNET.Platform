/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <vector>

#include "Types.h"

class Commander
{
    private:
        int m_file_descriptor;

        Commander* m_instance;

        std::vector<char>* m_Rx_Commander;
        std::vector<char>* m_Tx_Commander;

    public:
        Commander();
        ~Commander();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        Commander* getInstance();
};
