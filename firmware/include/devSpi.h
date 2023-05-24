//
// Author: Ice.Marek
// IceNET Technology 2023
//
#pragma once

#include "console.h"
#include "devBase.h"

#define MAX_SPI_DEVICES 2

class DevSpi : public DevBase
{
private:

	uint8_t m_id;

    typedef struct
    {
        int m_file_descriptor;
        unsigned char m_rx_buffer[32];
        uint8_t m_mode;
        uint8_t m_bits_per_word;
        uint32_t m_max_speed_hz;
    } SpiConfig;

    SpiConfig m_SpiConfig[MAX_SPI_DEVICES];

public:
	DevSpi();
	~DevSpi();
	
	//
	// Override functions
	//
	int device_open(const char* device) override;
	int device_read(uint8_t id) override;
	int device_write(uint8_t id) override;
	int device_close() override;
};