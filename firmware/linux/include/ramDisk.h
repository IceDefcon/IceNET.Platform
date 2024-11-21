/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2024
 *
 */

#ifndef RAM_DISK_H
#define RAM_DISK_H

/////////////////////////
//                     //
//                     //
//                     //
//    [RAM :: DISK]    //
//                     //
//                     //
//                     //
/////////////////////////

#define  RAM_0_DEVICE "KernelRam0"
#define  RAM_1_DEVICE "KernelRam1"

typedef enum
{
    DEVICE_RAM_DISK_0,
    DEVICE_RAM_DISK_1,
    DEVICE_RAM_DISK_AMOUNT
}ramDiskType;

typedef struct
{
    int ramDiskNumber;
    struct request_queue *ramDiskQueue;
    struct gendisk *ramDisk;
    struct list_head ramDiskList;
    spinlock_t ramDiskLock;
    struct radix_tree_root ramDiskPages;
}ramDiskDevice;

void ramDiskInit(void);
void ramDiskDestroy(void);

#endif // RAM_DISK_H
