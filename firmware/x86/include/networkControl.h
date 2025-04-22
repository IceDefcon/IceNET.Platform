/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#ifndef NETWORK_TRAFFIC_H
#define NETWORK_TRAFFIC_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/etherdevice.h>
#include <linux/inet.h>
#include <linux/printk.h>
#include <linux/if_arp.h>

typedef enum
{
    DATA_PACKET_UDP,
    DATA_PACKET_TCP,
    DATA_PACKET_AMOUNT,
}packetType;

/**
 * Ensuring the struct layout exactly
 * matches the ARP packet format as
 * defined by network protocols
 */
struct arpHeader
{
    __be16 ar_hrd;                  // Hardware type
    __be16 ar_pro;                  // Protocol type
    unsigned char ar_hln;           // Hardware address length
    unsigned char ar_pln;           // Protocol address length
    __be16 ar_op;                   // Opcode (request/reply)
    unsigned char ar_sha[ETH_ALEN]; // Sender MAC
    __be32 ar_sip;                  // Sender IP
    unsigned char ar_tha[ETH_ALEN]; // Target MAC
    __be32 ar_tip;                  // Target IP
} __attribute__((packed));

typedef struct
{
    struct net_device *networkDevice;
    const char *iface_name;
} networkControlType;

typedef struct
{
    struct sk_buff *socketBuffer;
    struct ethhdr *ethernetHeader;
    struct iphdr *ipHeader;
    struct udphdr *udpHeader;
    struct tcphdr *tcpHeader;
    unsigned char *Data[DATA_PACKET_AMOUNT];
    int broadcastLength;
    unsigned char destinationMAC[ETH_ALEN];
    __be32 source_IP;
    __be32 dest_IP;
} transferControlType;

int networkInit(void);
void networkDestroy(void);

#endif // NETWORK_TRAFFIC_H
