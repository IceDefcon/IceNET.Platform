/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#ifndef CHAR_DEVICE_H
#define CHAR_DEVICE_H

#include <linux/fs.h> // Include for file_operations struct
#include <linux/mutex.h> // Include for mutex functions

extern DEFINE_MUTEX(com_mutex);

int dev_release(struct inode *, struct file *);



#endif // CHAR_DEVICE_H