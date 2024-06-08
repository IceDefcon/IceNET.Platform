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

int main() 
{
	/* Heap Allocation */ 
    iceNET* iceNETinstance = new iceNET; /* TCP Server class */
    iceCOM* iceCOMinstance = new iceCOM; /* Kernel Communication */

	iceNETinstance->openDEV(); /* Open char device */
	iceCOMinstance->openDEV(); /* Open char device */

	while(true) /* Terminate Kernel comms and Clean Memory */
	{
	    if (iceCOMinstance->isThreadKilled()) 
	    {
	    	iceCOMinstance->closeDEV();
	    	iceNETinstance->closeDEV();
	        break;
	    }

	    /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete iceCOMinstance;
	delete iceNETinstance;

	return 0;
}
