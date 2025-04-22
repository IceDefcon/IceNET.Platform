/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#ifndef NETWORK_CONTROL_H
#define NETWORK_CONTROL_H

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>   // L2 - Network device structures (Ethernet)
#include <linux/skbuff.h>
#include <linux/ip.h>          // L3 - IP header
#include <linux/udp.h>         // L4 - UDP header
#include <linux/tcp.h>         // L4 - TCP header
#include <linux/etherdevice.h> // L2 - Ethernet header helpers
#include <linux/inet.h>
#include <linux/printk.h>
#include <linux/if_arp.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/if_ether.h>
#include <linux/crypto.h>
#include <linux/scatterlist.h>
#include <crypto/akcipher.h>
#include <linux/errno.h>
#include <linux/slab.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int networkInit(void);
void networkDestroy(void);

#endif // NETWORK_CONTROL_H
