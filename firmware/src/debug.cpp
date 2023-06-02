//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include "debug.h"

Debug::Debug() : m_LogLevel (LevelTodo) 
{
	Debug::Info("Debug :: Init Debug Console");
}

Debug::~Debug()
{
	Debug::Info("Debug :: Shutdown Debug Console");
}

void Debug::SetLevel(LogLevel level)
{
	m_LogLevel = level;
}

void Debug::Error(const char * message)
{
	if(m_LogLevel >= LevelError)
		std::cout << "[ERROR]: " << message << std::endl;
}

void Debug::Warning(const char * message)
{
	if(m_LogLevel >= LevelWarning)
		std::cout << "[WARNING]: " << message << std::endl;
}

void Debug::Info(const char * message)
{
	if(m_LogLevel >= LevelInfo)
		std::cout << "[INFO]: " << message << std::endl;
}

void Debug::Todo(const char * message)
{
	if(m_LogLevel >= LevelTodo)
		std::cout << "[TODO]: " << message << std::endl;
}

void Debug::Read(const char * message)
{
	std::cout << "[ RX ]: " << message << std::endl;
}

void Debug::Write(void)
{
	std::cout << "[ TX ]: ";
}