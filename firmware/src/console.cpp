//
// Author: Ice.Marek
// IceNET Technology 2023
//
#include "console.h"

Console::Console() : m_LogLevel (LevelTodo) 
{ 
	std::cout << "Console :: Construct" << std::endl;
	iceThread = std::thread(&Console::iceCOMThread, this);
}

Console::~Console()
{
	std::cout << "Console :: Destroy" << std::endl;

    if (iceThread.joinable()) 
    {
    	iceThread.join();
   	}
}

void 
Console::iceCOMThread()
{
    while (true) 
    {
        sem_wait(&wait_iceCOM);
        
        //////////////////
        // 				//
        // 				//
        // 				//
        // Thread  Work //
        // 				//
        // 				//
        // 				//
        //////////////////

        std::cout << "Testing exec" << std::endl;
    }
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
	std::cout << "[ RX ]: " << message << std::endl;
}

void Console::Write(void)
{
	std::cout << "[ TX ]: ";
}