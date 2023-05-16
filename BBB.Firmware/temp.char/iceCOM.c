#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define  DEVICE_NAME "FPGA_C_Device"
#define  CLASS_NAME  "FPGA_C_Device"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marek Ice");
MODULE_DESCRIPTION("A simple Linux device driver.");
MODULE_VERSION("1.0.0");

static int    majorNumber;
static char   message[256] = {0};
static short  size_of_message;
static int    numberOpens = 0;
static struct class*  C_Class  = NULL;
static struct device* C_Device = NULL;

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static DEFINE_MUTEX(com_mutex);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

static int __init chardev_init(void)
{
	printk(KERN_INFO "[FPGA][C] Initializing char device\n");

	// Try to dynamically allocate a major number for the device -- more difficult but worth it
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);

	if (majorNumber<0)
	{
		printk(KERN_ALERT "[FPGA][C] Failed to register major number\n");
		return majorNumber;
	}

	// Register the device class
	C_Class = class_create(THIS_MODULE, CLASS_NAME);
	
	if (IS_ERR(C_Class)) // Check for error and clean up if there is one
	{
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "[FPGA][C] Failed to register device class\n");
		return PTR_ERR(C_Class); // Correct way to return an error on a pointer
	}
	
	// Register the device driver
	C_Device = device_create(C_Class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	
	if (IS_ERR(C_Device)) 	// Clean up if there is an error
	{
		class_destroy(C_Class);           // Repeated code but the alternative is goto statements
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "[FPGA][C] Failed to create the device\n");
		return PTR_ERR(C_Device);
	}

	mutex_init(&com_mutex);       // Initialize the mutex lock dynamically at runtime

	return 0;
}
static void __exit chardev_exit(void)
{
	printk(KERN_INFO "[FPGA][C] Device Exit\n");
	device_destroy(C_Class, MKDEV(majorNumber, 0));     // remove the device
	class_unregister(C_Class);                          // unregister the device class
	class_destroy(C_Class);                             // remove the device class
	unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
	mutex_destroy(&com_mutex);        /// destroy the dynamically-allocated mutex
}

static int dev_open(struct inode *inodep, struct file *filep)
{
   if(!mutex_trylock(&com_mutex)) // Try to acquire the mutex (i.e., put the lock on/down)
   {
      // returns 1 if successful and 0 if there is contention
      printk(KERN_ALERT "[FPGA][C] Device in use by another process");
      return -EBUSY;
   }

	numberOpens++;
	printk(KERN_INFO "[FPGA][C] Device has been opened %d time(s)\n", numberOpens);
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int error_count = 0;
	// copy_to_user has the format ( * to, *from, size) and returns 0 on success
	error_count = copy_to_user(buffer, message, size_of_message);

	memset(message, 0, sizeof(message));

	if (error_count==0) // if true then have success
	{
		printk(KERN_INFO "[FPGA][C] Sent %d characters to the user\n", size_of_message);
		return (size_of_message=0);  // clear the position to the start and return 0
	}
	else 
	{
		printk(KERN_INFO "[FPGA][C] Failed to send %d characters to the user\n", error_count);
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
		printk(KERN_INFO "[FPGA][C] Received %d characters from the user\n", len);
		return len;
	} 
	else 
	{
		printk(KERN_INFO "[FPGA][C] Failed to receive characters from the user\n");
		return -EFAULT;
	}
}

static int dev_release(struct inode *inodep, struct file *filep)
{
	mutex_unlock(&com_mutex);  // Releases the mutex (i.e., the lock goes up)
	printk(KERN_INFO "[FPGA][C] Device successfully closed\n");
	return 0;
}

module_init(chardev_init);
module_exit(chardev_exit);
