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
    m_threadKill(false),
    m_readyKernel_OUT(false),
    m_TrafficRx(new std::vector<char>(NETWORK_TRAFFIC_SIZE)),
    m_TrafficTx(new std::vector<char>(NETWORK_TRAFFIC_SIZE)),
    m_instanceKernel_IN(nullptr)
{
    std::cout << "[INFO] [CONSTRUCTOR] Instantiate NetworkTraffic" << std::endl;

    /* Initialize m_TrafficRx and m_TrafficTx with zeros */
    std::fill(m_TrafficRx->begin(), m_TrafficRx->end(), 0);
    std::fill(m_TrafficTx->begin(), m_TrafficTx->end(), 0);
}

NetworkTraffic::~NetworkTraffic()
{
    std::cout << "[INFO] [DESTRUCTOR] Destroy NetworkTraffic" << std::endl;

    if (m_threadNetworkTraffic.joinable()) 
    {
        m_threadNetworkTraffic.join();
    }

    // delete m_TrafficRx;
    // delete m_TrafficTx;
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
    m_threadKill = true;

	return 0;
}


void NetworkTraffic::initThread()
{
    std::cout << "[INFO] [THREAD] Initialize State Machine" << std::endl;
    m_threadNetworkTraffic = std::thread(&NetworkTraffic::threadNetworkTraffic, this);
}


bool NetworkTraffic::isThreadKilled()
{
	return false;
}

void NetworkTraffic::threadNetworkTraffic()
{
    while(!m_threadKill) 
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
                    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_TrafficRx)[i]) << " ";
                }
                std::cout << std::endl;

                m_instanceKernel_IN->setTx_Kernel_IN(m_TrafficRx);

                setNetworkTrafficState(IDLE);
                break;

            case Kernel_OUT_TRANSFER:
                std::cout << "[INFO] [STM] Kernel_OUT_TRANSFER mode" << std::endl;
                m_readyKernel_OUT = true;
                /* TODO :: Temporary */
                m_threadKill = true;
                setNetworkTrafficState(IDLE);
                break;

            default:
                std::cout << "[INFO] [STM] Unknown mode" << std::endl;
        }


        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [THREAD] Terminate State Machine" << std::endl;
}

void NetworkTraffic::setNetworkTrafficState(stateType newState)
{
    m_currentState = newState;
}

void NetworkTraffic::setInstance_Kernel_IN(std::shared_ptr<Kernel_IN> instance)
{
    m_instanceKernel_IN = instance;
}

void NetworkTraffic::setNetworkTrafficRx(std::vector<char>* DataRx)
{
    m_TrafficRx = DataRx;
}

void NetworkTraffic::setNetworkTrafficTx(std::vector<char>* DataTx)
{
    m_TrafficTx = DataTx;
}

std::vector<char>* NetworkTraffic::getNetworkTrafficTx()
{
    return m_TrafficTx;
}

void NetworkTraffic::resetFeedbackFlag()
{
    m_readyKernel_OUT = false;
}

bool NetworkTraffic::getFeedbackFlag()
{
    return m_readyKernel_OUT;
}