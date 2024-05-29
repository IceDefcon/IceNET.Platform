/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#pragma once

#include "console.h"

class Core : public Console
{
public:

	Core()
	{
		Console::Info("[Core] Initialise the Core");
	}

	virtual ~Core()
	{
		Console::Info("[Core] Destroy the Core");
	}

	virtual int openCOM(const char* device) = 0;
	virtual int dataTX() = 0;
	virtual int dataRX() = 0;
	virtual int closeCOM() = 0;

private:

};

