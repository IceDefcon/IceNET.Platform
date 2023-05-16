#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include "console.h"

Console::Console() : m_LogLevel (LevelInfo) 
{ 
	Info("Enable Logging Console"); 
}

Console::~Console()
{

}

void Console::SetLevel(LogLevel level)
{
	m_LogLevel = level;
}

void Console::Error(const char * message)
{
	if(m_LogLevel >= LevelError)
		std::cout << "[ERROR]: " << message << std::endl;
}

void Console::Warning(const char * message)
{
	if(m_LogLevel >= LevelWarning)
		std::cout << "[WARNING]: " << message << std::endl;
}

void Console::Info(const char * message)
{
	if(m_LogLevel >= LevelInfo)
		std::cout << "[INFO]: " << message << std::endl;
}

void Console::Todo(const char * message)
{
	if(m_LogLevel >= LevelTodo)
		std::cout << "[TODO]: " << message << std::endl;
}

void Console::Read(const char * message)
{
	std::cout << "[ RX ]: " << message;
}

void Console::Write(void)
{
	std::cout << "[ TX ]: ";
}