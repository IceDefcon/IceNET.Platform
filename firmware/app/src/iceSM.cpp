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
    Info("[STM] Create iceSM object");
}

iceSM::~iceSM()
{
    Info("[STM] Destroy iceSM object");
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
    Info("[STM] Init the iceSMThread");
    m_iceSMThread = std::thread(&iceSM::iceSMThread, this);
}


bool iceSM::isThreadKilled()
{
	return false;
}

void iceSM::iceSMThread()
{
    Info("[STM] Enter iceSMThread");

    while(!m_killThread) 
    {
        switch(m_currentState)
        {
            case IDLE:
                // Info("[STM] IDLE mode");
                break;

            case CHAR_TO_TCP:
                Info("[STM] CHAR_TO_TCP mode");
                setStateMachine(IDLE);
                break;

            case TCP_TO_CHAR:
                Info("[STM] TCP_TO_CHAR mode");
                setStateMachine(IDLE);
                break;

            default:
                Info("[STM] Unknown mode");
        }


        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Info("[STM] Terminate iceSMThread");
}

void iceSM::setStateMachine(stateType newState)
{
    m_currentState = newState;
}