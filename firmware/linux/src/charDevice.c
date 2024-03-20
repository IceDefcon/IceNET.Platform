/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */





MODULE_LICENSE("GPL");

//////////////////////
//                  //
//                  //
//                  //
//    [C] Device    //
//                  //
//                  //
//                  //
//////////////////////



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