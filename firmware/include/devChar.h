//
// Author: Ice.Marek
// IceNET Technology 2023
//
#pragma once

#include "console.h"
#include "devBase.h"

#define BUFFER_LENGTH 256 

class DevChar : public Console, public DevBase
{
private:
	int m_file_descriptor;

public:

	DevChar();
	~DevChar();

	int device_open(const char* device) override;
	int device_read() override;
	int device_write() override;
	int device_close() override;
};