/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#ifndef CHAR_DEVICE_H
#define CHAR_DEVICE_H

#include <stdbool.h>
#include <stddef.h>

//////////////////////
//                  //
//                  //
//                  //
//    [C] Device    //
//                  //
//                  //
//                  //
//////////////////////

#define CD_OK 0

/* Feedback data to user-space */
static struct 
{
    char *data;
    size_t length;
    bool ready
} write_data;

/* Transfer data from user-space */
static struct transfer_data
{
    char *data;
    size_t length;
    bool ready;
} read_data;

/* GET TRANSFER DATA */ struct transfer_data* get_transfer_data(void);

void charDeviceInit(void);
void charDeviceDestroy(void);

#endif // CHAR_DEVICE_H