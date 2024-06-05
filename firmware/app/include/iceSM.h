/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#pragma once

#include <thread>
#include <atomic>
#include <iostream>

#include "iceCOM.h"
#include "iceNET.h"
#include "types.h"

class iceSM
{
	private:

		std::thread m_iceSMThread;
		std::atomic<bool> m_killThread;
		
		stateType m_currentState;

		iceCOM* m_iceCOMInstance;
		iceNET* m_iceNETInstance;

        /* For data Traffic */
        std::vector<char>* smRx;
        std::vector<char>* smTx;

	public:

		iceSM();
		~iceSM();

        int openCOM();
        int dataTX();
        int dataRX();
        int closeCOM();

        void initThread();
        bool isThreadKilled();
        void killThread();

		void iceSMThread();
		void setStateMachine(stateType newState);

		void setIceCOMinstance(iceCOM* instance);
		void setIceNETinstance(iceNET* instance);

};