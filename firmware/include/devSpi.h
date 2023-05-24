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
	int m_file_descriptor;
    unsigned char m_rx_buffer[32];
    uint8_t m_mode;
    uint8_t m_bits_per_word;
    uint32_t m_max_speed_hz;

public:
	DevSpi();
	~DevSpi();
	
	//
	// Override functions
	//
	int device_open(const char* device) override;
	int device_init(uint8_t mode, uint8_t bits_per_word, uint32_t max_speed_hz) override;
	int device_read() override;
	int device_write() override;
	int device_close() override;
};