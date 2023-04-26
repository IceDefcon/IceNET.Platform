

#include <linux/init.h>           	// Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         	// Core header for loading LKMs into the kernel
#include <linux/device.h>         	// Header to support the kernel Driver Model
#include <linux/kernel.h>         	// Contains types, macros, functions for the kernel
#include <linux/fs.h>             	// Header for the Linux file system support
#include <linux/uaccess.h>       	// Required for the copy to user function
#define  DEVICE_NAME "chardev"    	// The device will appear at /dev/ebbchar using this value
#define  CLASS_NAME  "ice"        	// The device class -- this is a character device driver

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marek Ice");
MODULE_DESCRIPTION("A simple Linux device driver.");
MODULE_VERSION("1.0.0");

static int    majorNumber;                  // Stores the device number -- determined automatically
static char   message[256] = {0};           // Memory for the string that is passed from userspace
static short  size_of_message;              // Used to remember the size of the string stored
static int    numberOpens = 0;              // Counts the number of times the device is opened
static struct class*  ebbcharClass  = NULL; // The device-driver class struct pointer
static struct device* ebbcharDevice = NULL; // The device-driver device struct pointer

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

static int __init chardev_init(void)
{
	printk(KERN_INFO "IceNET: Initializing the chardev\n");

	// Try to dynamically allocate a major number for the device -- more difficult but worth it
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber<0)
	{
		printk(KERN_ALERT "IceNET: chardev failed to register a major number\n");
		return majorNumber;
	}

	printk(KERN_INFO "IceNET: registered correctly with major number %d\n", majorNumber);

	// Register the device class
	ebbcharClass = class_create(THIS_MODULE, CLASS_NAME);
	
	if (IS_ERR(ebbcharClass)) // Check for error and clean up if there is one
	{
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "IceNET: Failed to register device class\n");
		return PTR_ERR(ebbcharClass); // Correct way to return an error on a pointer
	}
	
	printk(KERN_INFO "IceNET: device class registered correctly\n");

	// Register the device driver
	ebbcharDevice = device_create(ebbcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	
	if (IS_ERR(ebbcharDevice)) 	// Clean up if there is an error
	{
		class_destroy(ebbcharClass);           // Repeated code but the alternative is goto statements
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "IceNET: Failed to create the device\n");
		return PTR_ERR(ebbcharDevice);
	}
	
	printk(KERN_INFO "IceNET: device class created correctly\n"); // Made it! device was initialized
	
	return 0;
}
static void __exit chardev_exit(void)
{
	device_destroy(ebbcharClass, MKDEV(majorNumber, 0));     // remove the device
	class_unregister(ebbcharClass);                          // unregister the device class
	class_destroy(ebbcharClass);                             // remove the device class
	unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
	printk(KERN_INFO "IceNET: Goodbye from the LKM!\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
	numberOpens++;
	printk(KERN_INFO "IceNET: Device has been opened %d time(s)\n", numberOpens);
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int error_count = 0;
	// copy_to_user has the format ( * to, *from, size) and returns 0 on success
	error_count = copy_to_user(buffer, message, size_of_message);

	if (error_count==0) // if true then have success
	{
		printk(KERN_INFO "IceNET: Sent %d characters to the user\n", size_of_message);
		return (size_of_message=0);  // clear the position to the start and return 0
	}
	else 
	{
		printk(KERN_INFO "IceNET: Failed to send %d characters to the user\n", error_count);
		return -EFAULT; // Failed -- return a bad address message (i.e. -14)
	}
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	int error_count = 0;
	error_count = copy_from_user(message, buffer, len);

	if (error_count==0)
	{
		size_of_message = strlen(message); // store the length of the stored message
		printk(KERN_INFO "SendChar: Received %d characters from the user\n", len);
		return len;
	} 
	else 
	{
		printk(KERN_INFO "IceNET: Failed to receive characters from the user\n");
		return -EFAULT;
	}
}

static int dev_release(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "IceNET: Device successfully closed\n");
	return 0;
}

module_init(chardev_init);
module_exit(chardev_exit);
