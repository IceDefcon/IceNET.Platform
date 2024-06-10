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
#include "iceCOM.h"

class stateMachine
{
	private:

		std::thread m_stateMachineThread;
		std::atomic<bool> m_killThread;
		
		stateType m_currentState;

        /* For data Traffic */
        std::vector<char>* m_smRx;
        std::vector<char>* m_smTx;

        iceCOM* m_iceCOMinstance;

	public:

		stateMachine();
		~stateMachine();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        void initThread();
        bool isThreadKilled();

		void stateMachineThread();
		void setStateMachine(stateType newState);

        void setIceCOMinstance(iceCOM* instance);

		void setStateMachineRx(std::vector<char>* DataRx);
};