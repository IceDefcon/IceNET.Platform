#pragma once

#include "console.h"

using namespace std;

#define BUFFER_LENGTH 256 

class Device : public Console
{
private:

	int m_file_device;

public:

	Device();
	~Device();

	int device_open();
	int device_read();
	int device_write();
	int device_close();
};
