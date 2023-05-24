//
// Author: Ice.Marek
// IceNET Technology 2023
//
#pragma once

#define SPI_MAX_DEVICES 2
#define SPI_TX_BUFFER 8

class DevBase : public Console
{
public:

	virtual int device_open(const char* device) = 0;
	virtual int device_write(unsigned char tx_buffer[SPI_TX_BUFFER])() = 0;
	virtual int device_read() = 0;
	virtual int device_close() = 0;

private:

};

