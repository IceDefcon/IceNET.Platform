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
#include "stateMachine.h"

int main() 
{
	/* Heap Allocation */ 
    stateMachine* stateMachineinstance = new stateMachine;
    iceNET* iceNETinstance = new iceNET;
    iceCOM* iceCOMinstance = new iceCOM;

    stateMachineinstance->initSM(); /* Initialise State Machine */
	iceNETinstance->openDEV(); /* Open iceNET Device */
	iceCOMinstance->openDEV(); /* Open iceCOM Device */

	while(true) /* Terminate Kernel comms and Clean Memory */
	{
	    if (iceCOMinstance->isThreadKilled()) 
	    {
	    	iceCOMinstance->closeDEV();
	    	iceNETinstance->closeDEV();
	    	stateMachineinstance->shutdownSM();
	        break;
	    }

	    /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete stateMachineinstance;
	delete iceCOMinstance;
	delete iceNETinstance;

	return 0;
}
