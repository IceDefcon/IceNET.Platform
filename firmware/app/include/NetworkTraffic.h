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

#include "Types.h"
#include "Kernel_IN.h"

class NetworkTraffic
{
	private:

		std::thread m_NetworkTrafficThread;
		std::atomic<bool> m_killThread;
		
		stateType m_currentState;

        /* For data Traffic */
        std::vector<char>* m_smRx;
        std::vector<char>* m_smTx;

        Kernel_IN* m_Kernel_INinstance;

        bool m_Kernel_OUTfeedbackDataReady;

	public:

		NetworkTraffic();
		~NetworkTraffic();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        void initThread();
        bool isThreadKilled();

		void NetworkTrafficThread();
		void setNetworkTraffic(stateType newState);

        void setKernel_INinstance(Kernel_IN* instance);

		void setNetworkTrafficRx(std::vector<char>* DataRx);
		void setNetworkTrafficTx(std::vector<char>* DataTx);
		std::vector<char>* getNetworkTrafficTx();
		void resetFeedbackFlag();
		bool getFeedbackFlag();
};