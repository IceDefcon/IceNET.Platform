//
// Author: Ice.Marek
// IceNET Technology 2023
//
#pragma once
#include <iostream>
#include <thread>
#include <semaphore.h>



class Console
{
public:

	sem_t m_wait_iceCOM;
	
	enum LogLevel
	{
		LevelError = 0, LevelWarning, LevelInfo, LevelTodo
	};

private:

	LogLevel m_LogLevel;

	std::thread iceThread;

public:

	Console();
	~Console();

	void iceCOMThread();

	void SetLevel(LogLevel level);
	void Error(const char * message);
	void Warning(const char * message);
	void Info(const char * message);
	void Todo(const char * message);

	void Read(const char * message);
	void Write(void);
};