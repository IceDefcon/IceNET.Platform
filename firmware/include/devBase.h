//
// Author: Ice.Marek
// IceNET Technology 2023
//
#pragma once

class DevBase : public Console
{
public:

	DevBase()
	{
		std::cout << "DevBase :: Construct" << std::endl;
	}

	~DevBase()
	{
		std::cout << "DevBase :: Destroy" << std::endl;

	}
	
	virtual int device_open(const char* device) = 0;
	virtual int device_write() = 0;
	virtual int device_read() = 0;
	virtual int device_close() = 0;

private:

};

