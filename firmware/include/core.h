//
// Author: Ice.Marek
// IceNET Technology 2023
//
#pragma once

class Core : public Debug
{
public:

	Core()
	{
		Debug::Info("Core :: Init the Core");
	}

	virtual ~Core()
	{
		Debug::Info("Core :: Destroy the Core");
	}

	virtual int device_open(const char* device) = 0;
	virtual int device_write() = 0;
	virtual int device_read() = 0;
	virtual int device_close() = 0;

private:

};

