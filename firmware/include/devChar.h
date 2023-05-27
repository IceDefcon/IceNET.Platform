//
// Author: Ice.Marek
// IceNET Technology 2023
//
#pragma once

#include <thread>
#include <semaphore.h>
#include "console.h"
#include "devBase.h"

#define BUFFER_LENGTH 256 

class DevChar : public DevBase
{

private:

	int m_file_descriptor;

	std::thread iceThread;

	sem_t m_iceCOM_run;
	sem_t m_iceCOM_kill;

public:

	DevChar();
	~DevChar();

	void iceCOMThread();

	int device_open(const char* device) override;
	int device_read() override;
	int device_write() override;
	int device_close() override;

	void thread_run();
	void thread_kill();
};
