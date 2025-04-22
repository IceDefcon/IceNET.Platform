/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#ifndef NETWORK_H
#define NETWORK_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>          // L3 - IP header
#include <linux/udp.h>         // L4 - UDP header
#include <linux/tcp.h>         // L4 - TCP header
#include <linux/skbuff.h>
#include <linux/netdevice.h>   // L2 - Network device structures (Ethernet)
#include <linux/inet.h>
#include <linux/etherdevice.h> // L2 - Ethernet header helpers
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/crypto.h>
#include <linux/scatterlist.h>

#define UDP_PORT 54000
#define TCP_PORT 80

#define BROADCAST_IP 0xC0A808FF // 192.168.8.255

#define AES_KEY_SIZE 16
#define AES_BLOCK_SIZE 16

typedef struct
{
    struct ethhdr *ethernetHeader;
    struct iphdr *ipHeader;
    struct udphdr *udpHeader;
    struct tcphdr *tcpHeader;
    u8 *payload;
    int payloadLength;
    unsigned short destPort;
    u8 aesKey[AES_KEY_SIZE];
} hookControlType;

typedef struct
{
    struct arphdr *arpHeader;
    unsigned char *arpPtr;
    struct sk_buff *replySocketBuffer;
    struct net_device *networkDevice;
    unsigned char *senderMacAddress;
    unsigned char *senderIpAddress;
    unsigned char *targetMacAddress;
    unsigned char *targetIpAddress;
    unsigned char *arpReplyPtr;
    struct ethhdr *ethernetHeader;
    int arpHeaderLength;
    __be32 senderIp;
    __be32 targetIp;
    unsigned char ourMac[ETH_ALEN];
    __be32 ourIp;
    __be32 allowedSenderIp;
}arpRequestType;

void networkInit(void);
void networkDestroy(void);

#endif // NETWORK_H
