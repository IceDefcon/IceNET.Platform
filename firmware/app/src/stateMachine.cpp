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
    m_inputCOMinstance(nullptr),
    m_iceNETfeedbackDataReady(false)
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
                break;

            case inputCOM_TRANSFER:
                std::cout << "[INFO] [STM] inputCOM_TRANSFER mode" << std::endl;
                std::cout << "[INFO] [STM] Received 4 Bytes of data: ";
                for (int i = 0; i < 4; ++i)
                {
                    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_smRx)[i]) << " ";
                }
                std::cout << std::endl;

                m_inputCOMinstance->setinputCOMTx(m_smRx);

                setStateMachine(IDLE);
                break;

            case iceNET_TRANSFER:
                std::cout << "[INFO] [STM] iceNET_TRANSFER mode" << std::endl;
                m_iceNETfeedbackDataReady = true;
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

void stateMachine::setinputCOMinstance(inputCOM* instance)
{
    m_inputCOMinstance = instance;
}

void stateMachine::setStateMachineRx(std::vector<char>* DataRx)
{
    m_smRx = DataRx;
}

void stateMachine::setStateMachineTx(std::vector<char>* DataTx)
{
    m_smTx = DataTx;
}

std::vector<char>* stateMachine::getStateMachineTx()
{
    return m_smTx;
}

void stateMachine::resetFeedbackFlag()
{
    m_iceNETfeedbackDataReady = false;
}

bool stateMachine::getFeedbackFlag()
{
    return m_iceNETfeedbackDataReady;
}