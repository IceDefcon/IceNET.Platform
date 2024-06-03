/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#pragma once

#include <thread>
#include <atomic>

#include "types.h"
#include "console.h"

class iceSM : public Console
{
	private:

		std::thread m_iceSMThread;
		std::atomic<bool> m_killThread;
		
		stateType m_currentState;

	public:

		iceSM();
		~iceSM();

        int openCOM();
        int dataTX();
        int dataRX();
        int closeCOM();

        void initThread();
        bool isThreadKilled();

		void iceSMThread();
		void setStateMachine(stateType newState);
};