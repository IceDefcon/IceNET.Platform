/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <iostream>
#include <cstring>
#include <thread>
#include <unistd.h> // sleep
#include <chrono> // delay
#include <thread> // delay

#include "iceCOM.h"
#include "iceNET.h"
#include "console.h"

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        std::cerr << "[ERROR] Missing console parameter" << std::endl;
        return ERROR; // indicating error
    }

    /* Stack :: Interfaces pointer */
	Core* CoreClass = nullptr;

    if (strcmp(argv[1], "i2c") == 0) 
    {
        /* Initialise Kernel Communication */
        iceCOM* iceCOMinstance = new iceCOM;
        CoreClass = iceCOMinstance;
    } 
    else if (strcmp(argv[1], "tcp") == 0) 
    {
        /* Initialise Kernel Communication */
        iceNET* iceNETinstance = new iceNET(2555);
        CoreClass = iceNETinstance;
    } 
    else 
    {
        std::cerr << "[ERROR] Wrong console parameter" << std::endl;
        return ERROR;
    }

	CoreClass->openCOM();

	/**
	 * 
	 * 1. Terminate Program
	 * 2. Clean Memory 
	 * 
	 */
	while(true)
	{
	    if (CoreClass->terminate()) 
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

	    /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete CoreClass;

	return OK;
}
