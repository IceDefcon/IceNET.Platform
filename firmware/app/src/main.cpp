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
#include "iceSM.h"

int main() 
{
	/* Instantiation and Heap Allocation */ 
    iceSM* smInstance = new iceSM; /* Initialise Application State Machine */
    iceCOM* iceCOMinstance = new iceCOM; /* Initialise Kernel Communication */
    iceNET* iceNETinstance = new iceNET(2555); /* Initialise TCP Server Communication */

	/* Configure iceCOM and iceNET instances in State Machine*/
	smInstance->setIceCOMinstance(iceCOMinstance);
	smInstance->setIceNETinstance(iceNETinstance);

	/* Init Threads */
	smInstance->openCOM();
	iceCOMinstance->openCOM();
    iceNETinstance->openCOM();


	/* Terminate Kernel comms and Clean Memory */
	while(true)
	{
	    if (iceCOMinstance->isThreadKilled()) 
	    {
	    	iceCOMinstance->closeCOM();
	    	smInstance->killThread();
	    	/* TODO :: Thread is not killing dur to accept function */
			iceNETinstance->killThread();
	        break;
	    }

	    /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete iceNETinstance;
	delete iceCOMinstance;
	delete smInstance;

	return 0;
}
