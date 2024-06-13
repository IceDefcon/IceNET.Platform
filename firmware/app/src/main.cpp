/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <chrono> // delay
#include <thread> // delay

#include "Kernel_IN.h"
#include "Kernel_OUT.h"
#include "ServerTCP.h"
#include "NetworkTraffic.h"

int main() 
{
	/* Heap Allocation */ 
    Kernel_IN* Kernel_INinstance = new Kernel_IN;
    Kernel_OUT* Kernel_OUTinstance = new Kernel_OUT;
    ServerTCP* ServerTCPinstance = new ServerTCP;
    NetworkTraffic* NetworkTrafficinstance = new NetworkTraffic;

    /* Set Instances */
    Kernel_OUTinstance->setNetworkTrafficIstance(NetworkTrafficinstance);
    ServerTCPinstance->setNetworkTrafficIstance(NetworkTrafficinstance);
    NetworkTrafficinstance->setKernel_INinstance(Kernel_INinstance);

    /* Initialize Interfaces */
	Kernel_INinstance->openDEV();
	Kernel_OUTinstance->openDEV();
    ServerTCPinstance->openDEV();
    NetworkTrafficinstance->openDEV();

	while(true) /* Terminate Kernel comms and Clean Memory */
	{
	    if (Kernel_OUTinstance->isThreadKilled()) 
	    {
	    	ServerTCPinstance->closeDEV();
	    	Kernel_OUTinstance->closeDEV();
	    	Kernel_INinstance->closeDEV();
	    	NetworkTrafficinstance->closeDEV();
	        break;
	    }

	    /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete NetworkTrafficinstance;
	delete ServerTCPinstance;
	delete Kernel_OUTinstance;
	delete Kernel_INinstance;

	return 0;
}
