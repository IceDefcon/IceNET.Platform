/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#ifndef CHAR_DEVICE_H
#define CHAR_DEVICE_H

#include <linux/fs.h> // Include for file_operations struct

//////////////////////
//                  //
//                  //
//                  //
//    [C] Device    //
//                  //
//                  //
//                  //
//////////////////////

struct mutex *get_com_mutex(void);
struct file_operations *get_fops(void);

int dev_open(struct inode *inodep, struct file *filep);
ssize_t dev_read(struct file *, char *, size_t, loff_t *);
ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
int dev_release(struct inode *inodep, struct file *filep);






#endif // CHAR_DEVICE_H