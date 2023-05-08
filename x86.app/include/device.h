#ifndef DEVICE_H
#define DEVICE_H

using namespace std;

#define BUFFER_LENGTH 256 

class Device
{
	public:

	int m_file_device;

	int device_open();
	int device_read();
	int device_write();
	int device_exit();
};

#endif // DEVICE_H