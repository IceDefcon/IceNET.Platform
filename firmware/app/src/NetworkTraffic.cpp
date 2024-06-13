/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <chrono> // sleep_for
#include <thread> // sleep_for
#include <iomanip> // Include the <iomanip> header for setw and setfill

#include "NetworkTraffic.h"

NetworkTraffic::NetworkTraffic() :
    m_Kernel_INinstance(nullptr),
    m_Kernel_OUTfeedbackDataReady(false)
{
    std::cout << "[INFO] [CONSTRUCTOR] Instantiate NetworkTraffic" << std::endl;
}

NetworkTraffic::~NetworkTraffic()
{
    std::cout << "[INFO] [DESTRUCTOR] Destroy NetworkTraffic" << std::endl;
    if (m_NetworkTrafficThread.joinable()) 
    {
        m_NetworkTrafficThread.join();
    }
}

int NetworkTraffic::openDEV()
{
	initThread();

	return 0;
}

int NetworkTraffic::dataTX()
{
	return 0;
}

int NetworkTraffic::dataRX()
{
	return 0;
}

int NetworkTraffic::closeDEV()
{
    m_killThread = true;

	return 0;
}


void NetworkTraffic::initThread()
{
    std::cout << "[INFO] [THREAD] Initialize State Machine" << std::endl;
    m_NetworkTrafficThread = std::thread(&NetworkTraffic::NetworkTrafficThread, this);
}


bool NetworkTraffic::isThreadKilled()
{
	return false;
}

void NetworkTraffic::NetworkTrafficThread()
{
    while(!m_killThread) 
    {
        switch(m_currentState)
        {
            case IDLE:
                break;

            case Kernel_IN_TRANSFER:
                std::cout << "[INFO] [STM] Kernel_IN_TRANSFER mode" << std::endl;
                std::cout << "[INFO] [STM] Received 4 Bytes of data: ";
                for (int i = 0; i < 4; ++i)
                {
                    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_smRx)[i]) << " ";
                }
                std::cout << std::endl;

                m_Kernel_INinstance->setKernel_INTx(m_smRx);

                setNetworkTraffic(IDLE);
                break;

            case Kernel_OUT_TRANSFER:
                std::cout << "[INFO] [STM] Kernel_OUT_TRANSFER mode" << std::endl;
                m_Kernel_OUTfeedbackDataReady = true;
                /* TODO :: Temporary */
                m_killThread = true;
                setNetworkTraffic(IDLE);
                break;

            default:
                std::cout << "[INFO] [STM] Unknown mode" << std::endl;
        }


        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [THREAD] Terminate State Machine" << std::endl;
}

void NetworkTraffic::setNetworkTraffic(stateType newState)
{
    m_currentState = newState;
}

void NetworkTraffic::setKernel_INinstance(Kernel_IN* instance)
{
    m_Kernel_INinstance = instance;
}

void NetworkTraffic::setNetworkTrafficRx(std::vector<char>* DataRx)
{
    m_smRx = DataRx;
}

void NetworkTraffic::setNetworkTrafficTx(std::vector<char>* DataTx)
{
    m_smTx = DataTx;
}

std::vector<char>* NetworkTraffic::getNetworkTrafficTx()
{
    return m_smTx;
}

void NetworkTraffic::resetFeedbackFlag()
{
    m_Kernel_OUTfeedbackDataReady = false;
}

bool NetworkTraffic::getFeedbackFlag()
{
    return m_Kernel_OUTfeedbackDataReady;
}