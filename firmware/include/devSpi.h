//
// Author: Ice.Marek
// IceNET Technology 2023
//
#pragma once

#include "console.h"
#include "devBase.h"

class DevSpi : public DevBase
{
private:
	
	typedef struct
	{
		int m_file_descriptor;
	    unsigned char m_rx_buffer[8];
	    uint8_t m_mode;
	    uint8_t m_bits_per_word;
	    uint32_t m_max_speed_hz;
	} SpiConfig;

	SpiConfig m_SpiConfig[SPI_MAX_DEVICES];

public:
	DevSpi();
	~DevSpi();
	
	//
	// Override functions
	//
	int device_open(const char* device) override;
	int device_read() override;
	int device_write(unsigned char tx_buffer[SPI_TX_BUFFER]) override;
	int device_close() override;
};