//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include <iostream>
#include <thread>
#include <unistd.h> // sleep

#include "devChar.h"
#include "devSpi.h"
#include "console.h"

#define ICE "/dev/iceCOM"
#define SPI0 "/dev/spidev0.0"
#define SPI1 "/dev/spidev1.0"

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

	//
	// Sending text to Kernel :: Not linked with SPI
	//
	pDevice = new DevChar; 		// SLOW :: HEAP
	// DevChar CharDevice; 		// FAST :: STACK
	// pDevice = &CharDevice; 	// FAST :: STACK
	pDevice->device_open(ICE);
	pDevice->device_write();
	pDevice->device_read();
	pDevice->device_close();

	//
	// SPI 0
	//
	pDevice = new DevSpi; 		// SLOW :: HEAP
	// DevSpi SpiDevice; 		// FAST :: STACK
	// pDevice = &SpiDevice; 	// FAST :: STACK
	pDevice->device_open(SPI0);
	pDevice->device_init();
	pDevice->device_write();
	sleep(1);
	pDevice->device_read();
	pDevice->device_close();
	//
	// SPI 1
	//
	// pDevice->device_open(SPI1);
	// pDevice->device_init();
	// pDevice->device_write();
	// sleep(1);
	// pDevice->device_read();
	// pDevice->device_close();

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