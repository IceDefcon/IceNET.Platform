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
    m_currentState(NetworkTraffic_IDLE),
    m_readyKernelOutput(false),
    m_Rx_NetworkTraffic(new std::vector<char>(NETWORK_TRAFFIC_SIZE)),
    m_Tx_NetworkTraffic(new std::vector<char>(NETWORK_TRAFFIC_SIZE))
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate NetworkTraffic" << std::endl;

    /* Initialize m_Rx_NetworkTraffic and m_Tx_NetworkTraffic with zeros */
    std::fill(m_Rx_NetworkTraffic->begin(), m_Rx_NetworkTraffic->end(), 0);
    std::fill(m_Tx_NetworkTraffic->begin(), m_Tx_NetworkTraffic->end(), 0);
}

NetworkTraffic::~NetworkTraffic()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy NetworkTraffic" << std::endl;

    if (m_threadNetworkTraffic.joinable()) 
    {
        m_threadNetworkTraffic.join();
    }

    // delete m_Rx_NetworkTraffic;
    // delete m_Tx_NetworkTraffic;
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
    std::cout << "[INFO] [NET] Initialize threadNetworkTraffic" << std::endl;
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
            case NetworkTraffic_IDLE:
                break;

            case NetworkTraffic_KernelInput:
                std::cout << "[INFO] [NET] NetworkTraffic_KernelInput mode" << std::endl;
                std::cout << "[INFO] [NET] Received 4 Bytes of data: ";
                for (int i = 0; i < 4; ++i)
                {
                    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Rx_NetworkTraffic)[i]) << " ";
                }
                std::cout << std::endl;

                m_instanceKernelInput->setTx_KernelInput(m_Rx_NetworkTraffic);
                m_instanceKernelInput->setKernelInputState(KernelInput_TX);

                setNetworkTrafficState(NetworkTraffic_IDLE);

                break;

            case NetworkTraffic_KernelOutput:
                std::cout << "[INFO] [NET] NetworkTraffic_KernelOutput mode" << std::endl;
                m_readyKernelOutput = true;
                /* TODO :: Temporary */
                m_threadKill = true;
                setNetworkTrafficState(NetworkTraffic_IDLE);
                break;

            case NetworkTraffic_KILL:
                std::cout << "[INFO] [NET] NetworkTraffic_KILL mode" << std::endl;
                m_instanceKernelInput->setKernelInputState(KernelInput_KILL);
                setNetworkTrafficState(NetworkTraffic_IDLE);
                break;

            default:
                std::cout << "[INFO] [NET] Unknown mode" << std::endl;
        }


        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [NET] Terminate threadNetworkTraffic" << std::endl;
}

void NetworkTraffic::setNetworkTrafficState(NetworkTraffic_stateType newState)
{
    m_currentState = newState;
}

void NetworkTraffic::setInstance_KernelInput(const std::shared_ptr<KernelInput> instance)
{
    m_instanceKernelInput = instance;
}

void NetworkTraffic::setNetworkTrafficRx(std::vector<char>* DataRx)
{
    m_Rx_NetworkTraffic = DataRx;
}

void NetworkTraffic::setNetworkTrafficTx(std::vector<char>* DataTx)
{
    m_Tx_NetworkTraffic = DataTx;
}

std::vector<char>* NetworkTraffic::getNetworkTrafficTx()
{
    return m_Tx_NetworkTraffic;
}

void NetworkTraffic::resetFeedbackFlag()
{
    m_readyKernelOutput = false;
}

bool NetworkTraffic::getFeedbackFlag()
{
    return m_readyKernelOutput;
}