//
// Author: Ice.Marek
// IceNET Technology 2023
//
#pragma once

#include <thread>
#include <atomic>
#include <semaphore.h>
#include "console.h"
#include "devBase.h"

#define BUFFER_LENGTH 256 

class DevChar : public DevBase
{

private:
	int m_file_descriptor;
	std::thread iceThread;

	//
	// Atomic in case if case if
	// something interrupt atomic
	// processing of variable
	//
	std::atomic<bool> m_killThread;

public:

	DevChar();
	~DevChar() override;

	void iceCOMThread();

	int device_open(const char* device) override;
	int device_read() override;
	int device_write() override;
	int device_close() override;

	bool terminate();
};
