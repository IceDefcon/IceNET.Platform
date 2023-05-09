#pragma once  			// same as #ifndef
#include <iostream>

using namespace std;

class Console
{
public:

	enum LogLevel
	{
		LevelError = 0, LevelWarning, LevelInfo
	};

private:

	LogLevel m_LogLevel = LevelInfo; 	// Convetion m_xxx

public:

	Console();
	~Console();

	void SetLevel(LogLevel level);
	void Error(const char * message);
	void Warning(const char * message);
	void Info(const char * message);

	void Read(const char * message);
	void Write(void);
};