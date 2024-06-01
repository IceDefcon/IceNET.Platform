/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#pragma once

#include <iostream>

class Core
{
public:

	Core()
	{
		std::cout << "[INFO] [INIT] Initialise the Core" << std::endl;
	}

	virtual ~Core()
	{
		std::cout << "[INFO] [INIT] Destroy the Core" << std::endl;
	}

	virtual int openCOM() = 0;
	virtual int dataTX() = 0;
	virtual int dataRX() = 0;
	virtual int closeCOM() = 0;

    virtual void initThread() = 0;
    virtual bool isThreadKilled() = 0;

private:

};

