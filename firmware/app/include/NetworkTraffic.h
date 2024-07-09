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
#include <memory>
#include <iostream>

#include "KernelInput.h"
#include "Types.h"

class NetworkTraffic
{
	private:

		std::thread m_threadNetworkTraffic;
		std::atomic<bool> m_threadKill;
		NetworkTraffic_stateType m_currentState;
        bool m_readyKernelOutput;

        /* For data Traffic */
        std::vector<char>* m_Rx_NetworkTraffic;
        std::vector<char>* m_Tx_NetworkTraffic;

        std::shared_ptr<KernelInput> m_instanceKernelInput;

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
		void setNetworkTrafficState(NetworkTraffic_stateType newState);

        void setInstance_KernelInput(const std::shared_ptr<KernelInput> instance);

		void setNetworkTrafficRx(std::vector<char>* DataRx);
		void setNetworkTrafficTx(std::vector<char>* DataTx);
		std::vector<char>* getNetworkTrafficTx();
		void resetFeedbackFlag();
		bool getFeedbackFlag();
};