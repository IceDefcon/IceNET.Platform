/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <chrono> // delay
#include <thread> // delay

#include "inputCOM.h"
#include "outputCOM.h"
#include "serverTCP.h"
#include "linkCOM.h"

int main() 
{
	/* Heap Allocation */ 
    inputCOM* inputCOMinstance = new inputCOM;
    outputCOM* outputCOMinstance = new outputCOM;
    serverTCP* serverTCPinstance = new serverTCP;
    linkCOM* linkCOMinstance = new linkCOM;

    /* Set Instances */
    outputCOMinstance->setlinkCOMIstance(linkCOMinstance);
    serverTCPinstance->setlinkCOMIstance(linkCOMinstance);
    linkCOMinstance->setinputCOMinstance(inputCOMinstance);

    /* Initialize Interfaces */
	inputCOMinstance->openDEV();
	outputCOMinstance->openDEV();
    serverTCPinstance->openDEV();
    linkCOMinstance->openDEV();

	while(true) /* Terminate Kernel comms and Clean Memory */
	{
	    if (outputCOMinstance->isThreadKilled()) 
	    {
	    	serverTCPinstance->closeDEV();
	    	outputCOMinstance->closeDEV();
	    	inputCOMinstance->closeDEV();
	    	linkCOMinstance->closeDEV();
	        break;
	    }

	    /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete linkCOMinstance;
	delete serverTCPinstance;
	delete outputCOMinstance;
	delete inputCOMinstance;

	return 0;
}
