/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#pragma once

enum Status
{
	OK,
	ERROR,
	UNKNOWN
};

typedef enum 
{
    IDLE = 0,
    CHAR_TO_TCP,
    TCP_TO_CHAR,
} stateType;

