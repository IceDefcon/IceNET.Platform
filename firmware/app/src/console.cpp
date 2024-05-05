/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include "console.h"

Console::Console() : m_LogLevel (LevelTodo) 
{
	Console::Info("[Console] Initialise Debug Console");
}

Console::~Console()
{
	Console::Info("[Console] Terminate Debug Console");
}

void Console::SetLevel(LogLevel level)
{
	m_LogLevel = level;
}

void Console::Error(const char * message)
{
	if(m_LogLevel >= LevelError)
	{
		std::cout << "[ERNO] " << message << std::endl;
	}
}

void Console::Warning(const char * message)
{
	if(m_LogLevel >= LevelWarning)
	{
		std::cout << "[WARN] " << message << std::endl;
	}
}

void Console::Info(const char* message)
{
    if (m_LogLevel >= LevelInfo)
    {
        std::cout << "[INFO] " << message << std::endl;
    }
}

void Console::Info(const char * message, bool endLine)
{
	if(m_LogLevel >= LevelInfo)
	{
		std::cout << "[INFO] " << message;
		if(true == endLine) std::cout << std::endl;
	}
}

void Console::Todo(const char * message)
{
	if(m_LogLevel >= LevelTodo)
	{
		std::cout << "[TODO] " << message << std::endl;
	}
}

/*!
 * 
 * TODO
 * 
 * Read function may need to be
 * modified in order to take more
 * generic inputs
 * 
 */
void Console::Read(const char* message)
{
	Info("[ RX ] ",false);
    std::cout << "Feedback from Kernel[0] = 0x" << std::hex << static_cast<int>(message[0]) << std::endl;
}

void Console::Write(void)
{
	Info("[ TX ] ",false);
}