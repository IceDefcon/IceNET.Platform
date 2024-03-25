/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h> 	// Include for copy_to_user function
#include <linux/slab.h> 	// Include for kmalloc/kfree functions

#include "charDevice.h"
#include "workLoad.h"

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



