//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include <iostream> 		// IO devices :: keyboard
#include <fcntl.h> 			// Open device
#include <string.h> 		// for strlem
#include <unistd.h> 		// read/write to the file
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#include "devSpi.h"

DevSpi::DevSpi() 
{
	for (int i = 0; i < MAX_SPI_DEVICES; ++i)
	{
		m_SpiConfig[i].m_file_descriptor = 0;
		memset(m_SpiConfig[i].m_rx_buffer, 0, sizeof(m_SpiConfig[i].m_rx_buffer));
		m_SpiConfig[i].m_mode = 0;
		m_SpiConfig[i].m_bits_per_word = 0;
		m_SpiConfig[i].m_max_speed_hz = 0;
	}
}

DevSpi::~DevSpi() {}

int 
DevSpi::device_open(const char* device)
{
	m_SpiConfig[0].m_file_descriptor = open(device, O_RDWR);
	if (m_SpiConfig[0].m_file_descriptor < 0) return -1;

    // Configure SPI mode, bits per word, and max speed
    m_SpiConfig[0].m_mode = SPI_MODE_0;
	m_SpiConfig[0].m_bits_per_word = 8;
	m_SpiConfig[0].m_max_speed_hz = 1000000;
    int ret = ioctl(m_SpiConfig[0].m_file_descriptor, SPI_IOC_WR_MODE, &m_SpiConfig[0].m_mode);
    ret |= ioctl(m_SpiConfig[0].m_file_descriptor, SPI_IOC_WR_BITS_PER_WORD, &m_SpiConfig[0].m_bits_per_word);
    ret |= ioctl(m_SpiConfig[0].m_file_descriptor, SPI_IOC_WR_MAX_SPEED_HZ, &m_SpiConfig[0].m_max_speed_hz);
    if (ret < 0) return -1;

	return 0;
}

int 
DevSpi::device_read()
{
	// Print received data
    std::cout << "Received data:";
    for (int i = 0; i < sizeof(m_SpiConfig[0].m_rx_buffer); i++) 
    {
        std::cout << " 0x" << std::hex << (int)m_SpiConfig[0].m_rx_buffer[i];
    }
    std::cout << std::endl;

	return 0;
}

int DevSpi::device_write()
{
	unsigned char tx_buffer[32] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,};

    // Transfer data over SPI
    struct spi_ioc_transfer transfer = 
    {
    	.tx_buf = (unsigned long)tx_buffer,
    	.rx_buf = (unsigned long)m_SpiConfig[0].m_rx_buffer,
    	.len = sizeof(tx_buffer),
    	.speed_hz = m_SpiConfig[0].m_max_speed_hz,
    	.bits_per_word = m_SpiConfig[0].m_bits_per_word,
		// .delay_usecs = 0,
    };

    int ret = ioctl(m_SpiConfig[0].m_file_descriptor, SPI_IOC_MESSAGE(1), &transfer);
    if (ret < 0) return -1;
	
	return 0;
}

int DevSpi::device_close()
{
	close(m_SpiConfig[0].m_file_descriptor);

	return 0;
}