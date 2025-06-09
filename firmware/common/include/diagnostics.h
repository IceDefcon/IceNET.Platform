/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/list.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct active_host
{
    __be32 ip;
    unsigned char mac[ETH_ALEN];
    struct list_head list;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void initActiveHostList(void);
void cleanupActiveHostList(void);

void printActiveHosts(void);
void addActiveHost(__be32 ip, const unsigned char *mac);

#endif // DIAGNOSTICS_H
