/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <chrono> // delay
#include <thread> // delay

#include "iceCOM.h"
#include "iceNET.h"
#include "tcpServer.h"
#include "stateMachine.h"

int main() 
{
	/* Heap Allocation */ 
    iceCOM* iceCOMinstance = new iceCOM;
    iceNET* iceNETinstance = new iceNET;
    tcpServer* tcpServerinstance = new tcpServer;
    stateMachine* stateMachineinstance = new stateMachine;

    /* Set Instances */
    iceNETinstance->setStateMachineIstance(stateMachineinstance);
    tcpServerinstance->setStateMachineIstance(stateMachineinstance);
    stateMachineinstance->setIceCOMinstance(iceCOMinstance);

    /* Initialize Interfaces */
	iceCOMinstance->openDEV();
	iceNETinstance->openDEV();
    tcpServerinstance->openDEV();
    stateMachineinstance->openDEV();

	while(true) /* Terminate Kernel comms and Clean Memory */
	{
	    if (iceNETinstance->isThreadKilled()) 
	    {
	    	tcpServerinstance->closeDEV();
	    	iceNETinstance->closeDEV();
	    	iceCOMinstance->closeDEV();
	    	stateMachineinstance->closeDEV();
	        break;
	    }

	    /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete stateMachineinstance;
	delete tcpServerinstance;
	delete iceNETinstance;
	delete iceCOMinstance;

	return 0;
}
