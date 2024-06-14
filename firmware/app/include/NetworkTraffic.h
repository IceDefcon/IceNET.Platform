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

const size_t NETWORK_TRAFFIC_SIZE = 32;

class NetworkTraffic
{
	private:

		std::thread m_threadNetworkTraffic;
		std::atomic<bool> m_threadKill;
		stateType m_currentState;
        bool m_readyKernel_OUT;

        /* For data Traffic */
        std::vector<char>* m_Rx_NetworkTraffic;
        std::vector<char>* m_Tx_NetworkTraffic;

        std::shared_ptr<Kernel_IN> m_instanceKernel_IN;

	public:

		NetworkTraffic();
		~NetworkTraffic();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        void initThread();
        bool isThreadKilled();

		void threadNetworkTraffic();
		void setNetworkTrafficState(stateType newState);

        void setInstance_Kernel_IN(std::shared_ptr<Kernel_IN> instance);

		void setNetworkTrafficRx(std::vector<char>* DataRx);
		void setNetworkTrafficTx(std::vector<char>* DataTx);
		std::vector<char>* getNetworkTrafficTx();
		void resetFeedbackFlag();
		bool getFeedbackFlag();
};