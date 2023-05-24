//
// Author: Ice.Marek
// IceNET Technology 2023
//
#pragma once

class DevBase : public Console
{
public:

	virtual int device_open(const char* device) = 0;
	virtual int device_init(uint8_t mode, uint8_t bits_per_word, uint32_t max_speed_hz) = 0;
	virtual int device_write() = 0;
	virtual int device_read() = 0;
	virtual int device_close() = 0;

private:

};

