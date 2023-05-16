//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include <iostream>

#include "devChar.h"
#include "devSpi.h"
#include "console.h"

#define FPGA_DEVICE "/dev/FPGA"
#define SPI_DEVICE "/dev/spidev1.0"

int main(int argc, char* argv[])
{
	DevBase* pDevice;
	DevChar CharDevice;
	DevSpi SpiDevice;

	//
	// Sending to FPGA char device :: This is not linked with SPI device yet
	//
	// pDevice = &CharDevice;
	// pDevice->device_open(FPGA_DEVICE);
	// pDevice->device_write();
	// pDevice->device_read();
	// pDevice->device_close();

	//
	// Sending trought the SPI device to 
	//
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