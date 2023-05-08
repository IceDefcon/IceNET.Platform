//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include <iostream>

#include "device.h"

using namespace std;


int main()
{
	Device * pDevice = new Device;

	pDevice->device_open();
	pDevice->device_write();
	pDevice->device_read();



	return 0;
}
