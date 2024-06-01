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
#include "stateMachine.h"
#include "console.h"

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        std::cerr << "[ERROR] Missing console parameter" << std::endl;
        return ERROR;
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
        /* Initialise TCP Server Communication */
        iceNET* iceNETinstance = new iceNET(2555);
        CoreClass = iceNETinstance;
    }
    else if (strcmp(argv[1], "sm") == 0) 
    {
        /* Initialise Application State Machine */
        StateMachine* smInstance = new StateMachine;
        CoreClass = smInstance;
    } 
    else 
    {
        std::cerr << "[ERROR] Wrong console parameter" << std::endl;
        return ERROR;
    }

	CoreClass->openCOM();

	/* Terminate Instance and Clean Memory */
	while(true)
	{
		/**
		 * 
		 * TODO
		 * 
		 * Be aware that only iceCOM
		 * is terminated by "exit" 
		 * message from the
		 * input console
		 * 
		 * Flags for iceNET and StateMachine are not 
		 * computed, threads will not terminate
		 *
		 */
	    if (CoreClass->isThreadKilled()) 
	    {
	    	/* Close the core Device associated with instantiated class */
	    	CoreClass->closeCOM();
	        break;
	    }

	    /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete CoreClass;

	return OK;
}
