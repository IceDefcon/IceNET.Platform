/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include "debug.h"

Debug::Debug() : m_LogLevel (LevelTodo) 
{
	Debug::Info("[Debug] Initialise Debug Console");
}

Debug::~Debug()
{
	Debug::Info("[Debug] Terminate Debug Console");
}

void Debug::SetLevel(LogLevel level)
{
	m_LogLevel = level;
}

void Debug::Error(const char * message)
{
	if(m_LogLevel >= LevelError)
		std::cout << "[ERNO] " << message << std::endl;
}

void Debug::Warning(const char * message)
{
	if(m_LogLevel >= LevelWarning)
		std::cout << "[WARN] " << message << std::endl;
}

void Debug::Info(const char* message)
{
    if (m_LogLevel >= LevelInfo)
    {
        std::cout << "[INFO] " << message << std::endl;
    }
}

void Debug::Info(const char * message, bool endLine)
{
	if(m_LogLevel >= LevelInfo)
	{
		std::cout << "[INFO] " << message
		if(true == endLine ) std::cout << std::endl;
	}
}

void Debug::Todo(const char * message)
{
	if(m_LogLevel >= LevelTodo)
		std::cout << "[TODO] " << message << std::endl;
}

void Debug::Read(const char * message)
{
	Info("[ RX ]");
}

void Debug::Write(void)
{
	Info("[ TX ]",false);
}