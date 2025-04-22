/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#ifndef TRANSMITTER_H
#define TRANSMITTER_H

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

#define TARGET_IP    "192.168.8.174"
#define SRC_IP       "192.168.8.101"
#define DST_IP       "192.168.8.255"
#define SRC_PORT     12000
#define DST_PORT     54000
#define PAYLOAD_LEN  32

#define LISTEN_PORT 54000
#define BROADCAST_IP 0xC0A808FF // 192.168.8.255

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
    DATA_PACKET_UDP,
    DATA_PACKET_TCP,
    DATA_PACKET_AMOUNT,
}packetType;

typedef struct
{
    struct sk_buff *socketBuffer;
    struct ethhdr *ethernetHeader;
    struct iphdr *ipHeader;
    struct udphdr *udpHeader;
    struct tcphdr *tcpHeader;
    unsigned char *Data[DATA_PACKET_AMOUNT];
    int transmissionLength;
    unsigned char destinationMAC[ETH_ALEN];
    __be32 source_IP;
    __be32 dest_IP;
} transferControlType;

typedef struct
{
    struct net_device *networkDevice;
    const char *iface_name;
} networkControlType;

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* GET */ networkControlType* getNetworkController(void);

int udpTransmission(void);
int tcpTransmission(void);
int arpSendRequest(void);

#endif // TRANSMITTER_H
