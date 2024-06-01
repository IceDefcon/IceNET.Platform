/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#pragma once

#include <thread>
#include <atomic>

#include "core.h"
#include "console.h"

class StateMachine : public Core, public Console
{
	private:

		std::thread m_stateMachineThread;
		std::atomic<bool> m_killThread;


	public:

		StateMachine();
		~StateMachine();

        int openCOM() override;
        int dataTX() override;
        int dataRX() override;
        int closeCOM() override;

        void initThread() override;
        bool isThreadKilled() override;

		void stateMachineThread();
};