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
    iceCOM* iceCOMinstance = new iceCOM; /* Kernel Communication */
    iceNET* iceNETinstance = new iceNET; /* TCP Server class */

	iceCOMinstance->openDEV(); /* Open char device */
	iceNETinstance->openDEV(); /* Open char device */

	while(true) /* Terminate Kernel comms and Clean Memory */
	{
	    if (iceCOMinstance->isThreadKilled()) 
	    {
	    	iceCOMinstance->closeDEV();
	        break;
	    }

	    /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete iceCOMinstance;
	delete iceNETinstance;

	return 0;
}
