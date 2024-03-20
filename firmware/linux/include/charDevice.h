/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#ifndef CHAR_DEVICE_H
#define CHAR_DEVICE_H

#include <linux/fs.h> // Include for file_operations struct



ssize_t dev_read(struct file *, char *, size_t, loff_t *);




#endif // CHAR_DEVICE_H