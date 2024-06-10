/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <chrono> // sleep_for
#include <thread> // sleep_for
#include <iomanip> // Include the <iomanip> header for setw and setfill

#include "stateMachine.h"

stateMachine::stateMachine() :
    m_iceCOMinstance(nullptr)
{
    std::cout << "[INFO] [CONSTRUCTOR] Instantiate stateMachine" << std::endl;
}

stateMachine::~stateMachine()
{
    std::cout << "[INFO] [DESTRUCTOR] Destroy stateMachine" << std::endl;
    if (m_stateMachineThread.joinable()) 
    {
        m_stateMachineThread.join();
    }
}

int stateMachine::openDEV()
{
	initThread();

	return 0;
}

int stateMachine::dataTX()
{
	return 0;
}

int stateMachine::dataRX()
{
	return 0;
}

int stateMachine::closeDEV()
{
    m_killThread = true;

	return 0;
}


void stateMachine::initThread()
{
    std::cout << "[INFO] [THREAD] Initialize State Machine" << std::endl;
    m_stateMachineThread = std::thread(&stateMachine::stateMachineThread, this);
}


bool stateMachine::isThreadKilled()
{
	return false;
}

void stateMachine::stateMachineThread()
{
    while(!m_killThread) 
    {
        switch(m_currentState)
        {
            case IDLE:
                // std::cout << "[INFO] [STM] IDLE mode" << std::endl;
                //
                //
                //
                break;

            case iceCOM_TRANSFER:
                std::cout << "[INFO] [STM] iceCOM mode" << std::endl;
                std::cout << "[INFO] [STM] Received 4 Bytes of data: ";
                for (int i = 0; i < 4; ++i)
                {
                    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_smRx)[i]) << " ";
                }
                std::cout << std::endl;

                m_iceCOMinstance->setIceCOMTx(m_smRx);

                setStateMachine(IDLE);
                break;

            case iceNET_TRANSFER:
                std::cout << "[INFO] [STM] iceNET mode" << std::endl;
                //
                //
                //
                /* TODO :: Temporary */
                m_killThread = true;
                setStateMachine(IDLE);
                break;

            default:
                std::cout << "[INFO] [STM] Unknown mode" << std::endl;
        }


        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [THREAD] Terminate State Machine" << std::endl;
}

void stateMachine::setStateMachine(stateType newState)
{
    m_currentState = newState;
}

void stateMachine::setIceCOMinstance(iceCOM* instance)
{
    m_iceCOMinstance = instance;
}

// void stateMachine::setIceNETinstance(iceNET* instance)
// {
//     m_iceNETinstance = instance;
// }

void stateMachine::setStateMachineRx(std::vector<char>* DataRx)
{
    m_smRx = DataRx;
}