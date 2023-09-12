//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include <iostream>
#include <thread>
#include <unistd.h> // sleep

#include "iceCOM.h"
#include "debug.h"

#define ICE "/dev/iceCOM"

int main(void)
{
    //
    // HEAP
    //
	Core* CoreDevice = nullptr;
	iceCOM* iceCOMDevice = new iceCOM;
	//
	// Init Krenel Communication
	//
	CoreDevice = iceCOMDevice;
	CoreDevice->device_open(ICE);
	//
	// Main Comms
	//
	while(true)
	{
		//
		// Kill everything
		//
		if (iceCOMDevice != nullptr) 
		{
		    if (iceCOMDevice->terminate()) 
		    {
		        break;
		    }
		}
	}

	delete iceCOMDevice;

	return OK;
}

//////////////////////////////////////////
// 										//
// 		----===[ STRINGS ]===---- 		//
// 										//
//////////////////////////////////////////
// 										//
// char name[2] 	= {'0xA','0x7'}; 	//
// char name[] 		= "some string"; 	//
// const char* name = "some string"; 	//
// std::string name = "some string"; 	//
// 										//
//////////////////////////////////////////