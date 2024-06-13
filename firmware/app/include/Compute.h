/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#pragma once

#include <stdint.h>

#include "Console.h"

class Compute : public Console
{
	private:


	public:
		Compute();
		~Compute();

		uint8_t computeDeviceAddress(const char* in);
		uint8_t computeRegisterAddress(const char* in);
		uint8_t computeRegisterControl(const char* in);
		uint8_t computeRegisterData(const char* in);

};