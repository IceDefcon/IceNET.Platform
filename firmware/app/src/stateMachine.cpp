/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <chrono>           // sleep_for
#include <thread>           // sleep_for

#include "stateMachine.h"


StateMachine::StateMachine()
{

}

StateMachine::~StateMachine()
{

}




int StateMachine::openCOM()
{
	initThread();

	return 0;
}

int StateMachine::dataTX()
{
	return 0;
}

int StateMachine::dataRX()
{
	return 0;
}

int StateMachine::closeCOM()
{
	return 0;
}


void StateMachine::initThread()
{
    Console::Info("[COM] Init the stateMachineThread");
    m_stateMachineThread = std::thread(&StateMachine::stateMachineThread, this);
}


bool StateMachine::isThreadKilled()
{
	return false;
}

void StateMachine::stateMachineThread()
{
    Console::Info("[COM] Enter stateMachineThread");

    while(!m_killThread) 
    {
        if(true)
        {
            /* TODO */
        }
        else
        {
        	/* TODO */
        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::seconds(1));
    	Console::Info("[COM] stateMachineThread is Running");
    }

    Console::Info("[COM] Terminate stateMachineThread");
}
