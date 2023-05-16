#include <iostream>
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define SPI_DEVICE "/dev/spidev1.0"

int main()
{
    int fd;
    unsigned char tx_buffer[3] = {0x01, 0x02, 0x03};
    unsigned char rx_buffer[3] = {0};

    fd = open(SPI_DEVICE, O_RDWR);
    if (fd < 0) {
        std::cerr << "Failed to open SPI device: " << SPI_DEVICE << std::endl;
        return -1;
    }

    // Configure SPI mode, bits per word, and max speed
    uint8_t mode = SPI_MODE_0;
    uint8_t bits_per_word = 8;
    uint32_t max_speed_hz = 1000000;
    int ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    ret |= ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word);
    ret |= ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &max_speed_hz);
    if (ret < 0) {
        std::cerr << "Failed to configure SPI device" << std::endl;
        return -1;
    }

    // Transfer data over SPI
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx_buffer,
        .rx_buf = (unsigned long)rx_buffer,
        .len = sizeof(tx_buffer),
        .speed_hz = max_speed_hz,
        .bits_per_word = bits_per_word,
//        .delay_usecs = 0,
    };
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
    if (ret < 0) {
        std::cerr << "Failed to transfer data over SPI" << std::endl;
        return -1;
    }

    // Print received data
    std::cout << "Received data:";
    for (int i = 0; i < sizeof(rx_buffer); i++) {
        std::cout << " 0x" << std::hex << (int)rx_buffer[i];
    }
    std::cout << std::endl;

    close(fd);

    return 0;
}
