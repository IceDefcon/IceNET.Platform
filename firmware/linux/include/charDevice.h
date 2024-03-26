/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#ifndef CHAR_DEVICE_H
#define CHAR_DEVICE_H

//////////////////////
//                  //
//                  //
//                  //
//    [C] Device    //
//                  //
//                  //
//                  //
//////////////////////

#if 0 /* Do I need them ??? */
/* GET */ struct file_operations *get_fops(void);
/* GET */ struct mutex *get_com_mutex(void);
/* GET */ int *get_majorNumber(void);
/* SET */ void set_majorNumber(int major);
/* GET */ struct class *get_C_Class(void);
/* SET */ void set_C_Class(struct class *class);
/* GET */ struct device *get_C_Device(void);
/* SET */ void gst_C_Device(struct device *device);
#endif

int charDeviceInit(void);
void charDeviceDestroy(void);

#endif // CHAR_DEVICE_H