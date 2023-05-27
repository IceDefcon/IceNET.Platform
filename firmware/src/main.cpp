//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include <iostream>
#include <thread>
#include <unistd.h> // sleep

#include "devChar.h"
#include "console.h"

#define ICE "/dev/iceCOM"

int main(void)
{
    //
    // HEAP allocation
    //
	DevBase* pDevice = nullptr;
	DevChar* pCharDevice = new DevChar;
	//
	// Init Krenel Communication
	//
	pDevice = pCharDevice;
	pDevice->device_open(ICE);
	//
	// Main Comms
	//
	dynamic_cast<DevChar*>(pDevice)->device_post();

	for(;;)
		
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