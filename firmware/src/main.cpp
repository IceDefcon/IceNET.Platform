//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include <iostream>
#include <thread>
#include <unistd.h> // sleep
#include <semaphore.h>

#include "devChar.h"
#include "console.h"

#define ICE "/dev/iceCOM"

static bool run = true;




int main(int argc, char* argv[])
{
	//
    // Init wait semaphore :: Permit number 0
    // And join to ensure thread will execute
    // before program terminae
    //
    sem_init(&wait_iceCOM, 0, 0);
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
	while(run)
	{
		pDevice->device_write();
		sem_post(&wait_iceCOM);
		pDevice->device_read();
	}

    iceThread.join();
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