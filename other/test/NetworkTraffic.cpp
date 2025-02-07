/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */

#include <chrono> // sleep_for
#include <thread> // sleep_for
#include <iomanip> // Include the <iomanip> header for setw and setfill

#include "NetworkTraffic.h"

NetworkTraffic::NetworkTraffic() :
    m_threadKill(false),
    m_currentState(NetworkTraffic_IDLE),
    m_readyOutput(false),
    m_Rx_NetworkTraffic(new std::vector<char>(NETWORK_TRAFFIC_SIZE)),
    m_Tx_NetworkTraffic(new std::vector<char>(NETWORK_TRAFFIC_SIZE)),
    m_Rx_bytesReceived(0),
    m_Tx_bytesReceived(0)
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate NetworkTraffic" << std::endl;
}

NetworkTraffic::~NetworkTraffic()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy NetworkTraffic" << std::endl;

    if (m_threadNetworkTraffic.joinable()) 
    {
        m_threadNetworkTraffic.join();
    }
}

void NetworkTraffic::initBuffers()
{
    std::cout << "[INFO] [NET] Initialise NetworkTraffic Buffers" << std::endl;
    std::fill(m_Rx_NetworkTraffic->begin(), m_Rx_NetworkTraffic->end(), 0);
    std::fill(m_Tx_NetworkTraffic->begin(), m_Tx_NetworkTraffic->end(), 0);
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

            case NetworkTraffic_Input:
                std::cout << "[INFO] [NET] NetworkTraffic_Input mode" << std::endl;
                std::cout << "[INFO] [NET] Received 8 Bytes of data: ";
                for (int i = 0; i < 8; ++i)
                {
                    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Rx_NetworkTraffic)[i]) << " ";
                }
                std::cout << std::endl;

                m_instanceInput->setTx_Input(m_Rx_NetworkTraffic, m_Rx_bytesReceived);
                m_instanceInput->setInputState(Input_TX);

                setNetworkTrafficState(NetworkTraffic_IDLE);

                break;

            case NetworkTraffic_Output:
                std::cout << "[INFO] [NET] NetworkTraffic_Output mode" << std::endl;
                std::cout << "[INFO] [NET] Received " << m_Rx_bytesReceived << " Byte of data: ";
                for (int i = 0; i < m_Rx_bytesReceived; ++i)
                {
                    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Tx_NetworkTraffic)[i]) << " ";
                }
                std::cout << std::endl;
                m_readyOutput = true;
                setNetworkTrafficState(NetworkTraffic_IDLE);
                break;

            case NetworkTraffic_KILL:
                std::cout << "[INFO] [NET] NetworkTraffic_KILL mode" << std::endl;
                m_instanceInput->setInputState(Input_KILL);
                setNetworkTrafficState(NetworkTraffic_IDLE);
                m_threadKill = true;
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

void NetworkTraffic::setInstance_Input(const std::shared_ptr<Input> instance)
{
    m_instanceInput = instance;
}

void NetworkTraffic::setNetworkTrafficRx(std::vector<char>* DataRx, int bytesReceived)
{
    m_Rx_NetworkTraffic = DataRx;
    m_Rx_bytesReceived = bytesReceived;
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
    m_readyOutput = false;
}

bool NetworkTraffic::getFeedbackFlag()
{
    return m_readyOutput;
}
