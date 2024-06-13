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
#include "inputCOM.h"

class stateMachine
{
	private:

		std::thread m_stateMachineThread;
		std::atomic<bool> m_killThread;
		
		stateType m_currentState;

        /* For data Traffic */
        std::vector<char>* m_smRx;
        std::vector<char>* m_smTx;

        inputCOM* m_inputCOMinstance;

        bool m_iceNETfeedbackDataReady;

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

        void setinputCOMinstance(inputCOM* instance);

		void setStateMachineRx(std::vector<char>* DataRx);
		void setStateMachineTx(std::vector<char>* DataTx);
		std::vector<char>* getStateMachineTx();
		void resetFeedbackFlag();
		bool getFeedbackFlag();
};