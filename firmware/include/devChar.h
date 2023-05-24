//
// Author: Ice.Marek
// IceNET Technology 2023
//
#pragma once

#include "console.h"
#include "devBase.h"

#define BUFFER_LENGTH 256 

class DevChar : public DevBase
{
private:
	int m_file_descriptor;

public:

	DevChar();
	~DevChar();

	int device_open(const char* device) override;
	int device_read(uint8_t id) override;
	int device_write(uint8_t id) override;
	int device_close() override;};
