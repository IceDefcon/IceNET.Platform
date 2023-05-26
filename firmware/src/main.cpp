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

void ConsoleThread() 
{
	//
    // TODO
    //
    std::cout << "Thread 1 is executing." << std::endl;
}

int main(int argc, char* argv[])
{
	DevBase* pDevice = nullptr;
	// Allocate on HEAP
	DevChar* pCharDevice = new DevChar;

	// Sending text to Kernel space
	pDevice = pCharDevice;
	pDevice->device_open(ICE);
	pDevice->device_write();
	pDevice->device_read();
	pDevice->device_close();

	delete pCharDevice;

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