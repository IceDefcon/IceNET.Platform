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

std::counting_semaphore<int> wait_iceCOM(0);

void iceCOMTHread()
{
    while (true) 
    {
    	wait_iceCOM.acquire(); 

        std::cout << " iceCOM Work to do" << std::endl;
    }
}

int main(int argc, char* argv[])
{
	int ret;

	//
    // Create iceCOMThread and join
    // to ensure thread will execute
    // before program terminae
    //
    std::thread iceThread(iceCOMTHread);
    iceThread.join();
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
	ret = pDevice->device_write();
	
	if(-2 == ret)
	{
		std::cout << "Closing Application" << std::endl;
		pDevice->device_close();
		delete pCharDevice;
	}
	else wait_iceCOM.release();
	 
	pDevice->device_read();

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