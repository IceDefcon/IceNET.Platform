#ifndef DEVICE_H
#define DEVICE_H

using namespace std;

#define BUFFER_LENGTH 256 

class Device
{
	private:

	int m_file_device;

	public:

	int device_open();
	int device_read();
	int device_write();
	int device_close();
};

#endif // DEVICE_H