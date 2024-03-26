/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

#include <linux/mutex.h>

#include "mutex.h"

DEFINE_MUTEX(com_mutex);

struct mutex *get_com_mutex(void)
{
    return &com_mutex;
}