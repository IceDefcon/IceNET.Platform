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

class linkCOM
{
	private:

		std::thread m_linkCOMThread;
		std::atomic<bool> m_killThread;
		
		stateType m_currentState;

        /* For data Traffic */
        std::vector<char>* m_smRx;
        std::vector<char>* m_smTx;

        inputCOM* m_inputCOMinstance;

        bool m_outputCOMfeedbackDataReady;

	public:

		linkCOM();
		~linkCOM();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        void initThread();
        bool isThreadKilled();

		void linkCOMThread();
		void setlinkCOM(stateType newState);

        void setinputCOMinstance(inputCOM* instance);

		void setlinkCOMRx(std::vector<char>* DataRx);
		void setlinkCOMTx(std::vector<char>* DataTx);
		std::vector<char>* getlinkCOMTx();
		void resetFeedbackFlag();
		bool getFeedbackFlag();
};