/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <chrono> // delay
#include <thread> // delay

#include "core.h"
#include "iceCOM.h"
#include "iceNET.h"
#include "iceSM.h"

int main() 
{
    /* Stack :: Interfaces pointer */
	// Core* CoreClass = nullptr;

    // iceNET* iceNETinstance = new iceNET(2555); /* Initialise TCP Server Communication */
    // iceNETinstance->openCOM();

    // iceSM* smInstance = new iceSM; /* Initialise Application State Machine */
	// smInstance->openCOM();

    iceCOM* iceCOMinstance = new iceCOM; /* Initialise Kernel Communication */
	iceCOMinstance->openCOM();

    iceNET* iceNETinstance = new iceNET(2555); /* Initialise TCP Server Communication */
    iceNETinstance->openCOM();

	/* Terminate Kernel comms and Clean Memory */
	while(true)
	{
	    if (iceCOMinstance->isThreadKilled()) 
	    {
	    	iceCOMinstance->closeCOM();
	        break;
	    }

	    /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete iceCOMinstance;
	delete iceNETinstance;
	// delete smInstance;

	return 0;
}
