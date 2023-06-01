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
    // Allocate on HEAP
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
		// Kill everything if flag is set
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

	return 0;
}

//////////////////////////////////////////
//
// 		----===[ STRINGS ]===----
//
//////////////////////////////////////////
//
// char name[8] 	= {'0xD','0xE','0xA','0xD','0xC','0x0','0xD','0xE'}
// char name[] 		= "some string";
// const char* name = "some string";
// std::string name = "some string";
//
//////////////////////////////////////////