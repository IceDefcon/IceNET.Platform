/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#ifndef RECEIVER_H
#define RECEIVER_H

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
#include <linux/ipv6.h>
#include <linux/icmpv6.h>
#include <linux/if_ether.h>
#include <linux/crypto.h>
#include <linux/scatterlist.h>
#include <crypto/akcipher.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <net/addrconf.h>       // for ipv6_chk_addr
#include <net/ip6_checksum.h>   // for csum_ipv6_magic
#include <linux/icmp.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define UDP_PORT 54000
#define TCP_PORT 80
#define BROADCAST_HEX_IP 0xC0A808FF // 192.168.8.255
#define AES_KEY_SIZE 16
#define AES_BLOCK_SIZE 16
#define ICMPV6_NEIGHBOR_SOLICITATION 135
#define ICMPV6_NEIGHBOR_ADVERTISEMENT 136
#define ND_NA_FLAG_ROUTER     0x80
#define ND_NA_FLAG_SOLICITED  0x40
#define ND_NA_FLAG_OVERRIDE   0x20

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    struct icmphdr *icmpHeader;
    struct icmphdr *icmpHeaderReply;
    struct sk_buff *socketBufferReply;
    struct iphdr *ipHeaderReply;
    struct ethhdr *ethernetHeader;
    struct ethhdr *ethernetHeaderReply;
    unsigned char *dataBuffer;
    int dataBufferLength;
    struct iphdr *ipHeader;
} hookDiagnosticsType;

typedef struct
{
    struct iphdr *ipHeader;
    struct udphdr *udpHeader;
    struct tcphdr *tcpHeader;
    u8 *payload;
    int payloadLength;
    unsigned short destPort;
    u8 aesKey[AES_KEY_SIZE];
} hookCommunicationType;

typedef struct
{
    struct arphdr *arpHeader;
    unsigned char *arpPtr;
    struct sk_buff *socketBufferReply;
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int receiverHookDiagnostic(void *priv, struct sk_buff *socketBuffer, const struct nf_hook_state *state);
unsigned int receiverHookCommunication(void *priv, struct sk_buff *socketBuffer, const struct nf_hook_state *state);
int arpReceive(struct sk_buff *socketBuffer, struct net_device *networkDevice, struct packet_type *packetType, struct net_device *originalDevice);
int ndpReceive(struct sk_buff *socketBuffer, struct net_device *networkDevice, struct packet_type *pt, struct net_device *orig_dev);

#endif // RECEIVER_H
