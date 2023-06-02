//
// Author: Ice.Marek
// IceNET Technology 2023
//
#pragma once
#include <iostream>

class Debug
{
public:

	enum LogLevel
	{
		LevelError = 0, LevelWarning, LevelInfo, LevelTodo
	};

private:

	LogLevel m_LogLevel;

public:

	Debug();
	~Debug();

	void SetLevel(LogLevel level);
	void Error(const char * message);
	void Warning(const char * message);
	void Info(const char * message);
	void Todo(const char * message);

	void Read(const char * message);
	void Write(void);
};