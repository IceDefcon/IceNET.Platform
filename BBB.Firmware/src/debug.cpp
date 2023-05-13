#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include "debug.h"

using namespace std;

Console::Console()
: m_LogLevel (LevelInfo)
{
	Info("Log Level: 2 [Info]");
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
		cout << "[ERROR]: " << message << endl;
}

void Console::Warning(const char * message)
{
	if(m_LogLevel >= LevelWarning)
		cout << "[WARNING]: " << message << endl;
}

void Console::Info(const char * message)
{
	if(m_LogLevel >= LevelInfo)
		cout << "[INFO]: " << message << endl;
}

void Console::Read(const char * message)
{
	cout << "[ RX ]: " << message;
}

void Console::Write(void)
{
	cout << "[ TX ]: ";
}