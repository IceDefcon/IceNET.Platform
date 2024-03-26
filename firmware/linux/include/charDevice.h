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

//////////////////////
//                  //
//                  //
//                  //
//    [C] Device    //
//                  //
//                  //
//                  //
//////////////////////
#define  DEVICE_NAME "iceCOM"
#define  CLASS_NAME  "iceCOM"

int    majorNumber;
char   message[256] = {0};
unsigned long  size_of_message;
int    numberOpens = 0;
struct class*  C_Class  = NULL;
struct device* C_Device = NULL;

int     dev_open(struct inode *, struct file *);
int     dev_release(struct inode *, struct file *);
ssize_t dev_read(struct file *, char *, size_t, loff_t *);
ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

DEFINE_MUTEX(com_mutex);

struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

int dev_open(struct inode *inodep, struct file *filep);
ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset);
ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset);
int dev_release(struct inode *inodep, struct file *filep);



#endif // CHAR_DEVICE_H