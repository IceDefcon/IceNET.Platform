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

// Application
#include "devSpi.h"

DevSpi::DevSpi() : m_file_descriptor(0) {}
DevSpi::~DevSpi() {}

int 
DevSpi::device_open(const char* device)
{
	m_file_descriptor = open(device, O_RDWR);
	if (m_file_descriptor < 0) return -1;

	return 0;
}

int 
DevSpi::device_init()
{
    // Configure SPI mode, bits per word, and max speed
    uint8_t mode = SPI_MODE_0;
    uint8_t bits_per_word = 8;
    uint32_t max_speed_hz = 1000000;

    int ret = ioctl(m_file_descriptor, SPI_IOC_WR_MODE, &mode);
    ret |= ioctl(m_file_descriptor, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word);
    ret |= ioctl(m_file_descriptor, SPI_IOC_WR_MAX_SPEED_HZ, &max_speed_hz);

    if (ret < 0) return -1;
	return 0;
}

int 
DevSpi::device_read()
{
	// Print received data
    std::cout << "Received data:";
    for (int i = 0; i < sizeof(rx_buffer); i++) 
    {
        std::cout << " 0x" << std::hex << (int)rx_buffer[i];
    }
    std::cout << std::endl;

	return 0;
}

int DevSpi::device_write()
{
    // Transfer data over SPI
    struct spi_ioc_transfer transfer = 
    {
    	.tx_buf = (unsigned long)tx_buffer,
    	.rx_buf = (unsigned long)rx_buffer,
    	.len = sizeof(tx_buffer),
    	.speed_hz = max_speed_hz,
    	.bits_per_word = bits_per_word,
		// .delay_usecs = 0,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
    if (ret < 0) return -1;
	
	return 0;
}

int DevSpi::device_close()
{
	close(m_file_descriptor);

	return 0;
}