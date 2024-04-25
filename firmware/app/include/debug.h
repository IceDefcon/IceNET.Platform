/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#pragma once
#include <iostream>
#include "types.h"

class Debug
{
public:

	enum LogLevel
	{
		LevelError = 0, 
		LevelWarning, 
		LevelInfo, 
		LevelTodo
	};

private:

	LogLevel m_LogLevel;

public:

	Debug();
	~Debug();

	// This is currently not in use !!
	void SetLevel(LogLevel level);

	void Error(const char * message);
	void Warning(const char * message);
	void Info(const char * message, bool endLine);
	void Todo(const char * message);

	void Read(const char * message);
	void Write(void);
};