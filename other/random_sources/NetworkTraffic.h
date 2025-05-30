/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <iostream>

#include "Input.h"
#include "Types.h"

class NetworkTraffic
{
	private:

		std::thread m_threadNetworkTraffic;
		std::atomic<bool> m_threadKill;
		NetworkTraffic_stateType m_currentState;
        bool m_readyOutput;

        /* For data Traffic */
        std::vector<char>* m_Rx_NetworkTraffic;
        std::vector<char>* m_Tx_NetworkTraffic;

        int m_Rx_bytesReceived;
        int m_Tx_bytesReceived;

        std::shared_ptr<Input> m_instanceInput;

	public:

		NetworkTraffic();
		~NetworkTraffic();

        void initBuffers();

        int openDEV();
        int dataTX();
        int dataRX();
        int closeDEV();

        void initThread();
        bool isThreadKilled();

		void threadNetworkTraffic();
		void setNetworkTrafficState(NetworkTraffic_stateType newState);

        void setInstance_Input(const std::shared_ptr<Input> instance);

        void setNetworkTrafficRx(std::vector<char>* DataRx, int bytesReceived);
		void setNetworkTrafficTx(std::vector<char>* DataTx);
		std::vector<char>* getNetworkTrafficTx();
		void resetFeedbackFlag();
		bool getFeedbackFlag();
};
