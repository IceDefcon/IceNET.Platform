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
	DevChar* pCharDevice = new DevChar;
	DevSpi* pSpiDevice = new DevSpi;

	//
	// Sending text to Kernel :: Not linked with SPI
	//
	pDevice = pCharDevice;
	pDevice->device_open(ICE);
	pDevice->device_write();
	pDevice->device_read();
	pDevice->device_close();

	//
	// SPI 0
	//
	pDevice = pSpiDevice;
	pDevice->device_open(SPI0);
	// This must be dynamically casted
	// to have access to the non virtual methods
	// that are out of the DevBase class
	DevSpi* pDevSpi = dynamic_cast<DevSpi*>(pDevice);
	int id = pDevSpi->device_getid();
	pDevice->device_write();
	sleep(1);
	pDevice->device_read();
	pDevice->device_close();
	//
	// SPI 1
	//
	pDevice->device_open(SPI1);
	pDevice->device_write();
	sleep(1);
	pDevice->device_read();
	pDevice->device_close();

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