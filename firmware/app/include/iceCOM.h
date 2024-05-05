/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include <semaphore.h>

#include "debug.h"
#include "core.h"


/**
 * 
 * Console command buffer
 * 
 * Buffer length is defined by:
 * Length of command string + null terminator
 * 
 * 32 is a safe value !
 * 
 */
const size_t BUFFER_LENGTH = 32;

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

	/* Dynamically allocated  memory */
	std::vector<char> charDeviceeRx(BUFFER_LENGTH);
	std::vector<char> charDeviceeTx(BUFFER_LENGTH);

	std::vector<char> consoleBuffer(BUFFER_LENGTH);

public:

	iceCOM();
	~iceCOM();

	void initThread();
	void iceCOMThread();

	int device_open(const char* device) override;
	int device_read() override;
	int device_write() override;
	int device_close() override;

	bool terminate();
};
