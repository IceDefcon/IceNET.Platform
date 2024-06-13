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
#include "serverTCP.h"
#include "stateMachine.h"

int main() 
{
	/* Heap Allocation */ 
    inputCOM* inputCOMinstance = new inputCOM;
    outputCOM* outputCOMinstance = new outputCOM;
    serverTCP* serverTCPinstance = new serverTCP;
    stateMachine* stateMachineinstance = new stateMachine;

    /* Set Instances */
    outputCOMinstance->setStateMachineIstance(stateMachineinstance);
    serverTCPinstance->setStateMachineIstance(stateMachineinstance);
    stateMachineinstance->setinputCOMinstance(inputCOMinstance);

    /* Initialize Interfaces */
	inputCOMinstance->openDEV();
	outputCOMinstance->openDEV();
    serverTCPinstance->openDEV();
    stateMachineinstance->openDEV();

	while(true) /* Terminate Kernel comms and Clean Memory */
	{
	    if (outputCOMinstance->isThreadKilled()) 
	    {
	    	serverTCPinstance->closeDEV();
	    	outputCOMinstance->closeDEV();
	    	inputCOMinstance->closeDEV();
	    	stateMachineinstance->closeDEV();
	        break;
	    }

	    /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete stateMachineinstance;
	delete serverTCPinstance;
	delete outputCOMinstance;
	delete inputCOMinstance;

	return 0;
}
