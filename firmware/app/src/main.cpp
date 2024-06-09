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
    tcpServer* tcpServerinstance = new tcpServer;
    stateMachine* stateMachineinstance = new stateMachine;
    iceNET* iceNETinstance = new iceNET;
    iceCOM* iceCOMinstance = new iceCOM;

    /* Set instances */
    tcpServerinstance->setStateMachineIstance(stateMachineinstance);
    stateMachineinstance->setIceCOMinstance(iceCOMinstance);
    stateMachineinstance->setIceNETinstance(iceNETinstance);

    /**
     * 
     * TODO
     * 
     * Core class inplementation
     * required for creadted
     * inherited objects
     * 
     */
    tcpServerinstance->openDEV();
    stateMachineinstance->openDEV(); /* Initialise State Machine */
	iceNETinstance->openDEV(); /* Open iceNET Device */
	iceCOMinstance->openDEV(); /* Open iceCOM Device */

	while(true) /* Terminate Kernel comms and Clean Memory */
	{
	    if (iceCOMinstance->isThreadKilled()) 
	    {
	    	iceCOMinstance->closeDEV();
	    	iceNETinstance->closeDEV();
	    	stateMachineinstance->closeDEV();
	    	/**
	    	 * 
	    	 * TODO
	    	 * 
	    	 * When this is executed TCP client must offload
	    	 * in order to release the accept function
	    	 * and join the iceNET Thread
	    	 * 
	    	 */
	    	tcpServerinstance->closeDEV();
	        break;
	    }

	    /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete stateMachineinstance;
	delete tcpServerinstance;
	delete iceCOMinstance;
	delete iceNETinstance;

	return 0;
}
