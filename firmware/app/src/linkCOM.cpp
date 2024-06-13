/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <chrono> // sleep_for
#include <thread> // sleep_for
#include <iomanip> // Include the <iomanip> header for setw and setfill

#include "linkCOM.h"

linkCOM::linkCOM() :
    m_inputCOMinstance(nullptr),
    m_outputCOMfeedbackDataReady(false)
{
    std::cout << "[INFO] [CONSTRUCTOR] Instantiate linkCOM" << std::endl;
}

linkCOM::~linkCOM()
{
    std::cout << "[INFO] [DESTRUCTOR] Destroy linkCOM" << std::endl;
    if (m_linkCOMThread.joinable()) 
    {
        m_linkCOMThread.join();
    }
}

int linkCOM::openDEV()
{
	initThread();

	return 0;
}

int linkCOM::dataTX()
{
	return 0;
}

int linkCOM::dataRX()
{
	return 0;
}

int linkCOM::closeDEV()
{
    m_killThread = true;

	return 0;
}


void linkCOM::initThread()
{
    std::cout << "[INFO] [THREAD] Initialize State Machine" << std::endl;
    m_linkCOMThread = std::thread(&linkCOM::linkCOMThread, this);
}


bool linkCOM::isThreadKilled()
{
	return false;
}

void linkCOM::linkCOMThread()
{
    while(!m_killThread) 
    {
        switch(m_currentState)
        {
            case IDLE:
                break;

            case inputCOM_TRANSFER:
                std::cout << "[INFO] [STM] inputCOM_TRANSFER mode" << std::endl;
                std::cout << "[INFO] [STM] Received 4 Bytes of data: ";
                for (int i = 0; i < 4; ++i)
                {
                    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_smRx)[i]) << " ";
                }
                std::cout << std::endl;

                m_inputCOMinstance->setinputCOMTx(m_smRx);

                setlinkCOM(IDLE);
                break;

            case outputCOM_TRANSFER:
                std::cout << "[INFO] [STM] outputCOM_TRANSFER mode" << std::endl;
                m_outputCOMfeedbackDataReady = true;
                /* TODO :: Temporary */
                m_killThread = true;
                setlinkCOM(IDLE);
                break;

            default:
                std::cout << "[INFO] [STM] Unknown mode" << std::endl;
        }


        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [THREAD] Terminate State Machine" << std::endl;
}

void linkCOM::setlinkCOM(stateType newState)
{
    m_currentState = newState;
}

void linkCOM::setinputCOMinstance(inputCOM* instance)
{
    m_inputCOMinstance = instance;
}

void linkCOM::setlinkCOMRx(std::vector<char>* DataRx)
{
    m_smRx = DataRx;
}

void linkCOM::setlinkCOMTx(std::vector<char>* DataTx)
{
    m_smTx = DataTx;
}

std::vector<char>* linkCOM::getlinkCOMTx()
{
    return m_smTx;
}

void linkCOM::resetFeedbackFlag()
{
    m_outputCOMfeedbackDataReady = false;
}

bool linkCOM::getFeedbackFlag()
{
    return m_outputCOMfeedbackDataReady;
}