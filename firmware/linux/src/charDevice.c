/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include "charDevice.h"
#include "workLoad.h"

//////////////////////
//                  //
//                  //
//                  //
//    [C] Device    //
//                  //
//                  //
//                  //
//////////////////////
int dev_open(struct inode *inodep, struct file *filep)
{
    if(!mutex_trylock(&com_mutex))
    {
        printk(KERN_ALERT "[FPGA][ C ] Device in use by another process");
        return -EBUSY;
    }

    numberOpens++;
    printk(KERN_INFO "[FPGA][ C ] Device has been opened %d time(s)\n", numberOpens);
    return NULL;
}

ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    //
    // Copy to user space :: *to, *from, size :: returns 0 on success
    //
    error_count = copy_to_user(buffer, message, size_of_message);
    memset(message, 0, sizeof(message));

    if (error_count==0)
    {
        printk(KERN_INFO "[FPGA][ C ] Sent %d characters to the user\n", size_of_message);
        return (size_of_message = 0);  // clear the position to the start and return NULL
    }
    else 
    {
        printk(KERN_INFO "[FPGA][ C ] Failed to send %d characters to the user\n", error_count);
        return -EFAULT; // Failed -- return a bad address message (i.e. -14)
    }
}

ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    error_count = copy_from_user(message, buffer, len);

    if(strncmp(message, "a", 1) == 0)
    {
        tx_fpga[0] = 0x0F;
        printk(KERN_INFO "ICE Debug 1");
        queue_work(fpga_wq, &fpga_work);
    }

    if (error_count==0)
    {
        size_of_message = strlen(message);
        printk(KERN_INFO "[FPGA][ C ] Received %d characters from the user\n", len);
        return len;
    } 
    else 
    {
        printk(KERN_INFO "[FPGA][ C ] Failed to receive characters from the user\n");
        return -EFAULT;
    }
}

/*!
 * 
 * Experimental Drone control
 * 
 */
// ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
// {
//     int error_count = 0;
//     error_count = copy_from_user(message, buffer, len);

//     memset(&Move, 0, sizeof(struct Direction));

//     switch (message[0]) {
//         case 'w':
//             Move.Up = true;
//             Move.Go = true;
//             break;

//         case 's':
//             Move.Down = true;
//             Move.Go = true;
//             break;

//         case 'a':
//             Move.Left = true;
//             Move.Go = true;
//             break;

//         case 'd':
//             Move.Right = true;
//             Move.Go = true;
//             break;

//         default:
//             break;
//     }

//     if (Move.Go)
//     {
//         Move.Go = false;
//         queue_work(fpga_wq, &fpga_work);
//     }

//     if (error_count==0)
//     {
//         size_of_message = strlen(message);
//         printk(KERN_INFO "[FPGA][ C ] Received %d characters from the user\n", len);
//         return len;
//     } 
//     else 
//     {
//         printk(KERN_INFO "[FPGA][ C ] Failed to receive characters from the user\n");
//         return -EFAULT;
//     }
// }

int dev_release(struct inode *inodep, struct file *filep)
{
    mutex_unlock(&com_mutex);
    printk(KERN_INFO "[FPGA][ C ] Device successfully closed\n");
    return NULL;
}