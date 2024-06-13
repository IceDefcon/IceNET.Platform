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
		std::cout << "[INFO] [CONSTRUCTOR] Instantiate Core" << std::endl;
	}

	virtual ~Core()
	{
		std::cout << "[INFO] [DESTRUCTOR] Destroy Core" << std::endl;
	}

	virtual int openDEV() = 0;
	virtual int dataTX() = 0;
	virtual int dataRX() = 0;
	virtual int closeDEV() = 0;

    virtual void initThread() = 0;
    virtual bool isThreadKilled() = 0;

private:

};

