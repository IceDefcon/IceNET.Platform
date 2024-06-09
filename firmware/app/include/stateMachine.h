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
#include <iostream>

#include "types.h"

class stateMachine
{
	private:

		std::thread m_stateMachineThread;
		std::atomic<bool> m_killThread;
		
		stateType m_currentState;

        /* For data Traffic */
        std::vector<char>* smRx;
        std::vector<char>* smTx;

	public:

		stateMachine();
		~stateMachine();

        int initSM();
        int dataTX();
        int dataRX();
        int shutdownSM();

        void initThread();
        bool isThreadKilled();

		void stateMachineThread();
		void setStateMachine(stateType newState);
};