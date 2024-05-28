/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <iostream>
#include <thread>
#include <unistd.h> // sleep

#include "iceCOM.h"
#include "iceNET.h"
#include "console.h"

#define ICE "/dev/iceCOM"

int main(void)
{
    /**
     * 
     * Stack Allocation pointer
     * for switching between
     * iceCOM and iceNET
     * 
     */
	Core* CoreClass = nullptr;

	/**
	 * 
	 * Heap allocation of the
	 * communication interfaces
	 * 
	 */
	iceCOM* iceCOMDevice = new iceCOM; /* char Device */
	iceNET* iceNETServer = new iceNET(2555); /* tcp Server */

	/* Initialise Kernel Communication */
	CoreClass = iceCOMDevice;
	CoreClass->startCOM(ICE);
	/**
	 * 
	 * TODO
	 * 
	 * Initialise TCP
	 * listen and accept
	 * 
	 */

	/**
	 * 
	 * 1. Terminate Program
	 * 2. Clean Memory 
	 * 
	 */
	while(true)
	{
		if (iceCOMDevice != nullptr) 
		{
		    if (iceCOMDevice->terminate()) 
		    {
		    	/**
		    	 * 
		    	 * Shutdown Kernel Communication
		    	 * 
		    	 * 1. Atomic Thread Kill
		    	 * 2. Close the core Device associated with the class
		    	 * 
		    	 */
		    	CoreClass->closeCOM();
		        break;
		    }
		}
	}

	delete iceCOMDevice;
	delete iceNETServer;

	return OK;
}
