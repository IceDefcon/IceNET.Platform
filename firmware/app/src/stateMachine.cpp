/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <chrono> // sleep_for
#include <thread> // sleep_for

#include "stateMachine.h"

stateMachine::stateMachine()
{
    std::cout << "[INFO] [CONSTRUCTOR] Initialise stateMachine Object" << std::endl;
}

stateMachine::~stateMachine()
{
    std::cout << "[INFO] [DESTRUCTOR] Destroy stateMachine Object" << std::endl;
    if (m_stateMachineThread.joinable()) 
    {
        m_stateMachineThread.join();
    }
}

int stateMachine::initSM()
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

int stateMachine::shutdownSM()
{
    m_killThread = true;

	return 0;
}


void stateMachine::initThread()
{
    std::cout << "[INFO] [STM] Init the stateMachineThread" << std::endl;
    m_stateMachineThread = std::thread(&stateMachine::stateMachineThread, this);
}


bool stateMachine::isThreadKilled()
{
	return false;
}

void stateMachine::stateMachineThread()
{
    std::cout << "[INFO] [STM] Enter stateMachineThread" << std::endl;

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
                //
                //
                //
                setStateMachine(IDLE);
                break;

            case iceNET_TRANSFER:
                std::cout << "[INFO] [STM] iceNET mode" << std::endl;
                //
                //
                //
                setStateMachine(IDLE);
                break;

            default:
                std::cout << "[INFO] [STM] Unknown mode" << std::endl;
        }


        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "[INFO] [STM] stateMachineThread is Running" << std::endl;
    }

    std::cout << "[INFO] [STM] Terminate stateMachineThread" << std::endl;
}

void stateMachine::setStateMachine(stateType newState)
{
    m_currentState = newState;
}
