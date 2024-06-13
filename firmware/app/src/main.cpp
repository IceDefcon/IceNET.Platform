/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <chrono> // delay
#include <thread> // delay

#include "inputCOM.h"
#include "outputCOM.h"
#include "tcpServer.h"
#include "stateMachine.h"

int main() 
{
	/* Heap Allocation */ 
    inputCOM* inputCOMinstance = new inputCOM;
    outputCOM* outputCOMinstance = new outputCOM;
    tcpServer* tcpServerinstance = new tcpServer;
    stateMachine* stateMachineinstance = new stateMachine;

    /* Set Instances */
    outputCOMinstance->setStateMachineIstance(stateMachineinstance);
    tcpServerinstance->setStateMachineIstance(stateMachineinstance);
    stateMachineinstance->setinputCOMinstance(inputCOMinstance);

    /* Initialize Interfaces */
	inputCOMinstance->openDEV();
	outputCOMinstance->openDEV();
    tcpServerinstance->openDEV();
    stateMachineinstance->openDEV();

	while(true) /* Terminate Kernel comms and Clean Memory */
	{
	    if (outputCOMinstance->isThreadKilled()) 
	    {
	    	tcpServerinstance->closeDEV();
	    	outputCOMinstance->closeDEV();
	    	inputCOMinstance->closeDEV();
	    	stateMachineinstance->closeDEV();
	        break;
	    }

	    /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete stateMachineinstance;
	delete tcpServerinstance;
	delete outputCOMinstance;
	delete inputCOMinstance;

	return 0;
}
