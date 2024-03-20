/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#ifndef CHAR_DEVICE_H
#define CHAR_DEVICE_H

#include <linux/fs.h> 		//
#include <linux/device.h> 	//
#include <linux/mutex.h>  	//
#include <linux/uaccess.h> 	// copy_to_user

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

static int    majorNumber;
static char   message[256] = {0};
static unsigned long  size_of_message;
static int    numberOpens = 0;
static struct class*  C_Class  = NULL;
static struct device* C_Device = NULL;

int     dev_open(struct inode *, struct file *);
int     dev_release(struct inode *, struct file *);
ssize_t dev_read(struct file *, char *, size_t, loff_t *);
ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static DEFINE_MUTEX(com_mutex);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

// Export symbols
EXPORT_SYMBOL(dev_open);
EXPORT_SYMBOL(dev_read);
EXPORT_SYMBOL(dev_write);
EXPORT_SYMBOL(dev_release);

#endif // CHAR_DEVICE_H