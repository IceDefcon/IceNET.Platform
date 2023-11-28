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


const size_t BUFFER_LENGTH = 100;

class iceCOM : public Core
{

private:
	int m_file_descriptor;
	std::thread m_iceThread;

	/*!
	 * 
	 * Atomic in case if something 
	 * decide to interrupt kill 
	 * flag processing
	 * 
	 */
	std::atomic<bool> m_killThread;

public:

	iceCOM();
	iceCOM(size_t BUFFER_LENGTH);
	~iceCOM();

	void initThread();
	void iceCOMThread();

	int device_open(const char* device) override;
	int device_read() override;
	int device_write() override;
	int device_close() override;

	bool terminate();
};
