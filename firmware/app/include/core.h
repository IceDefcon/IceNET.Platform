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
		Console::Info("[INIT] Initialise the Core");
	}

	virtual ~Core()
	{
		Console::Info("[DESTROY] Destroy the Core");
	}

	virtual int openCOM() = 0;
	virtual int dataTX() = 0;
	virtual int dataRX() = 0;
	virtual int closeCOM() = 0;

    virtual void initThread() = 0;
    virtual bool isThreadKilled() = 0;

private:

};

