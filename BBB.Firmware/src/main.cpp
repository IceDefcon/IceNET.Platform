//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include <iostream>

// #include "devBase.h"
#include "devChar.h"
#include "devSpi.h"
#include "console.h"

int main(int argc, char* argv[])
{
	if(argv[1] == NULL)
	{
		std::cout << "Missing device, try with /dev/FPGA" << std::endl;
		return 0;
	}

	DevBase* pDevice;
	
	DevChar CharDevice;
	DevSpi SpiDevice;

	pDevice = &CharDevice;

	pDevice->device_open(argv[1]);
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
// char name[] 		= "some string";
// const char* name = "some string";
// std::string name = "some string";
//
//////////////////////////////////////////