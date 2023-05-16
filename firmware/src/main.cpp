//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include <iostream>

#include "devChar.h"
#include "devSpi.h"
#include "console.h"

#define ICE_DEVICE "/dev/iceCOM"
#define SPI_DEVICE "/dev/spidev1.0"

int main(int argc, char* argv[])
{
	DevBase* pDevice;

	//
	// Sending to FPGA char device :: This is not linked with SPI device yet
	//
	DevChar CharDevice;
	pDevice = &CharDevice;
	pDevice->device_open(ICE_DEVICE);
	pDevice->device_write();
	pDevice->device_read();
	pDevice->device_close();

	//
	// Testing SPI 
	//
	DevSpi SpiDevice;
	pDevice = &SpiDevice;
	pDevice->device_open(SPI_DEVICE);
	pDevice->device_write();
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