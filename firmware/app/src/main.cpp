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
#include "console.h"

#define ICE "/dev/iceCOM"

int main(void)
{
    /* Stack Allocation */
	Core* CoreDevice = nullptr;

	/* Heap Allocation */
	iceCOM* iceCOMDevice = new iceCOM;

	/* Initialise Kernel Communication */
	CoreDevice = iceCOMDevice;
	CoreDevice->device_open(ICE);

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
		    	CoreDevice->device_close();
		        break;
		    }
		}
	}

	delete iceCOMDevice;

	return OK;
}
