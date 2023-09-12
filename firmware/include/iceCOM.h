//
// Author: Ice.Marek
// IceNET Technology 2023
//
#pragma once

#include <thread>
#include <atomic>
#include <semaphore.h>

#include "debug.h"
#include "core.h"

class iceCOM : public Core
{

private:
	int m_file_descriptor;
	std::thread m_iceThread;

	//
	// Atomic in case if something 
	// decide to interrupt kill 
	// flag processing
	//
	std::atomic<bool> m_killThread;

	const size_t m_BUFFER_LENGTH;

public:

	iceCOM();
	iceCOM(size_t m_BUFFER_LENGTH);
	~iceCOM();

	void iceCOMThread();

	int device_open(const char* device) override;
	int device_read() override;
	int device_write() override;
	int device_close() override;

	bool terminate();
};
