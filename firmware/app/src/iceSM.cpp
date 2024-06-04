/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <chrono>           // sleep_for
#include <thread>           // sleep_for

#include "iceSM.h"

iceSM::iceSM()
{
    std::cout << "[INFO] [CONSTRUCTOR] Initialise iceSM Object" << std::endl;
}

iceSM::~iceSM()
{
    std::cout << "[INFO] [DESTRUCTOR] Destroy iceSM Object" << std::endl;
    if (m_iceSMThread.joinable()) 
    {
        m_iceSMThread.join();
    }
}

int iceSM::openCOM()
{
	initThread();

	return 0;
}

int iceSM::dataTX()
{
	return 0;
}

int iceSM::dataRX()
{
	return 0;
}

int iceSM::closeCOM()
{
	return 0;
}


void iceSM::initThread()
{
    std::cout << "[INFO] [STM] Init the iceSMThread" << std::endl;
    m_iceSMThread = std::thread(&iceSM::iceSMThread, this);
}


bool iceSM::isThreadKilled()
{
	return false;
}

void iceSM::killThread()
{
    m_killThread = true;
}

void iceSM::iceSMThread()
{
    std::cout << "[INFO] [STM] Enter iceSMThread" << std::endl;

    while(!m_killThread) 
    {
        switch(m_currentState)
        {
            case IDLE:
                if(true == m_iceNETInstance->getDataReady())
                {
                    setStateMachine(TCP_TO_CHAR);
                    m_iceNETInstance->setDataReady(false);
                }
                else if(true == m_iceCOMInstance->getDataReady())
                {
                    setStateMachine(CHAR_TO_TCP);
                    m_iceCOMInstance->setDataReady(false);
                }
                break;

            case CHAR_TO_TCP:
                std::cout << "[INFO] [STM] CHAR_TO_TCP mode" << std::endl;
                setStateMachine(IDLE);
                break;

            case TCP_TO_CHAR:
                std::cout << "[INFO] [STM] TCP_TO_CHAR mode" << std::endl;
                setStateMachine(IDLE);
                break;

            default:
                std::cout << "[INFO] [STM] Unknown mode" << std::endl;
        }


        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [STM] Terminate iceSMThread" << std::endl;
}

void iceSM::setStateMachine(stateType newState)
{
    m_currentState = newState;
}

void iceSM::setIceCOMinstance(iceCOM* instance)
{
    m_iceCOMInstance = instance;
}

void iceSM::setIceNETinstance(iceNET* instance)
{
    m_iceNETInstance = instance;
}
