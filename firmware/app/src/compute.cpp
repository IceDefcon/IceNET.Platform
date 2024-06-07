/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include "compute.h"

Compute::Compute() 
{
	Info("[CONSTRUCTOR] Instantiate Compute");
}

Compute::~Compute() 
{
	Info("[DESTRUCTOR] Destroy Compute");
}

uint8_t Compute::computeDeviceAddress(const char* in)
{
    uint8_t temp[2] = {0x00};
    uint8_t out = 0x00;

    temp[0] = in[0];
    temp[1] = in[1];

    if(temp[0] >= 0x30 && temp[0] <= 0x37)
    {
        out = (temp[0] - 0x30) << 4;
    }
    else
    {
        Error("[COM] Bad device address :: Max 7-bits");
        return 0xFF;
    }

    if(temp[1] >= 0x30 && temp[1] <= 0x39)
    {
        out = out + temp[1] - 0x30;
    }
    else if(temp[1] >= 0x61 && temp[1] <= 0x66)
    {
        out = out + temp[1] - 0x61 + 0x0A;
    }
    else
    {
        Error("[COM] Bad device address :: Max 7-bits");
        return 0xFF;
    }

    return out;
}

uint8_t Compute::computeRegisterAddress(const char* in)
{
    uint8_t temp[3] = {0x00};
    uint8_t out = 0x00;

    temp[0] = in[2];
    temp[1] = in[3];
    temp[2] = in[4];

    if(temp[0] == 0x20) /* Check for ASCII space */ 
    {
        if(temp[1] >= 0x30 && temp[1] <= 0x37)
        {
            out = (temp[1] - 0x30) << 4;
        }
        else
        {
            Error("[COM] Register Not Found");
            return 0xFF;
        }

        if(temp[2] >= 0x30 && temp[2] <= 0x39)
        {
            out = out + temp[2] - 0x30;
        }
        else if(temp[2] >= 0x61 && temp[2] <= 0x66)
        {
            out = out + temp[2] - 0x61 + 0x0A;
        }
        else
        {
            Error("[COM] Register Not Found");
            return 0xFF;
        }
    }
    else
    {
        Error("[COM] No space between DevicAaddress & RegisterAddress");
        return 0xFF;
    }

    return out;
}

uint8_t Compute::computeRegisterControl(const char* in)
{
    uint8_t temp[2] = {0x00};
    uint8_t out = 0x00;

    temp[0] = in[5];
    temp[1] = in[6];

    if(temp[0] == 0x20) /* Check for ASCII space */ 
    {
        if(temp[1] == 0x72) out = 0x00; /* Read */
        else if(temp[1] == 0x77) out = 0x01; /* Write */
        else
        {
            Error("[COM] Bad R/W operator");
            return 0xFF;
        }
    }
    else
    {
        Error("[COM] No space between RegisterAddress & R/W operator");
        return 0xFF;
    }

    return out;
}

uint8_t Compute::computeRegisterData(const char* in)
{
    uint8_t temp[3] = {0x00};
    uint8_t out = 0x00;

    temp[0] = in[7];
    temp[1] = in[8];
    temp[2] = in[9];

    if(temp[0] == 0x20) /* Check for ASCII space */ 
    {
        if(temp[1] >= 0x30 && temp[1] <= 0x39)
        {
            out = (temp[1] - 0x30) << 4;
        }
        else if(temp[1] >= 0x61 && temp[1] <= 0x66)
        {
            out = (temp[1] - 0x61 + 0x0A) << 4;
        }
        else
        {
            Error("[COM] Invalid Write Data");
            return 0xFF;
        }

        if(temp[2] >= 0x30 && temp[2] <= 0x39)
        {
            out = out + temp[2] - 0x30;
        }
        else if(temp[2] >= 0x61 && temp[2] <= 0x66)
        {
            out = out + temp[2] - 0x61 + 0x0A;
        }
        else
        {
            Error("[COM] Invalid Write Data");
            return 0xFF;
        }
    }
    else
    {
        Error("[COM] No space between R/W operator & RegisterData");
        return 0xFF;
    }

    return out;
}
