//
// Author: Ice.Marek
// IceNET Technology 2023
//
#pragma once
#include <iostream>
#include <thread>

sem_t wait_iceCOM;

class Console
{
public:

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

	void iceCOMTHread();

	void SetLevel(LogLevel level);
	void Error(const char * message);
	void Warning(const char * message);
	void Info(const char * message);
	void Todo(const char * message);

	void Read(const char * message);
	void Write(void);
};