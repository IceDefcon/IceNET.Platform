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

#include "console.h"
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
#define CHAR_DEVICE_BUFFER = 32;
#define CONSOLE_BUFFER = 32;

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

	/* Buffers for char Device transfer */
	std::vector<char> charDeviceeRx(CHAR_DEVICE_BUFFER);
	std::vector<char> charDeviceeTx(CHAR_DEVICE_BUFFER);
	/* Buffer for the console character input */
	std::vector<char> consoleBuffer(CONSOLE_BUFFER);

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
