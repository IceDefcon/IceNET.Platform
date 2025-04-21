/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */

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
#include <linux/crypto.h>
#include <linux/scatterlist.h>
#include <linux/printk.h>

#include "transmiter.h"

#define SRC_IP       "192.168.8.101"
#define DST_IP       "192.168.8.255"
#define SRC_PORT     12345
#define DST_PORT     54321
#define PAYLOAD_LEN  32
#define AES_KEY_LEN  16
#define AES_BLOCK_SIZE 16

// [L7] Application Layer: Static payload encryption key
static const unsigned char aes_key[AES_KEY_LEN] =
{
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x97, 0x75, 0x46, 0x7e, 0x56, 0x4e
};

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
    int broadcastLength;
    unsigned char destinationMAC[ETH_ALEN];
    __be32 source_IP;
    __be32 dest_IP;
} transferControlType;

static transferControlType transferControl =
{
    .socketBuffer = NULL,
    .ethernetHeader = NULL,
    .ipHeader = NULL,
    .udpHeader = NULL,
    .tcpHeader = NULL,
    .Data =
    {
        [DATA_PACKET_UDP] = NULL,
        [DATA_PACKET_TCP] = NULL,
    },
    .broadcastLength = 0,
    .destinationMAC = {0},
    .source_IP = 0,
    .dest_IP = 0
};

typedef struct
{
    struct net_device *networkDevice;
    char *iface_name;

}networkControlType;

static networkControlType networkControl =
{
    .networkDevice = NULL,
    .iface_name = "wlp2s0",
};


typedef struct
{
    const char *src_ip;
    const char *dst_ip;
    const u8 *dst_mac;
    __be16 src_port;
    __be16 dst_port;
    const u8 *payload;
    size_t payload_len;
}transmissionControlType;


static int aesEncrypt(void *payloadData, size_t len, u8 *key, u8 *iv) // [L6] Presentation Layer: AES block encryption of payload
{
    struct crypto_cipher *tfm;
    int i;

    if (len % AES_BLOCK_SIZE != 0)
    {
        pr_err("[TX][L6] Data length must be a multiple of 16 bytes\n");
        return -EINVAL;
    }

    tfm = crypto_alloc_cipher("aes", 0, 0);
    if (IS_ERR(tfm))
    {
        pr_err("[TX][L6] Failed to allocate AES cipher\n");
        return PTR_ERR(tfm);
    }

    if (crypto_cipher_setkey(tfm, key, AES_KEY_LEN))
    {
        pr_err("[TX][L6] Failed to set AES key\n");
        crypto_free_cipher(tfm);
        return -EIO;
    }

    for (i = 0; i < len; i += AES_BLOCK_SIZE)
    {
        crypto_cipher_encrypt_one(tfm, payloadData + i, payloadData + i);
    }

    crypto_free_cipher(tfm);

    return 0;
}

int udpBroadcastTransmission(void)
{
    transferControl.broadcastLength = ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr) + PAYLOAD_LEN;

    // [L2] Data Link Layer: Set destination to broadcast MAC
    memset(transferControl.destinationMAC, 0xFF, ETH_ALEN);

    // [L3] Network Layer
    transferControl.source_IP = in_aton(SRC_IP);
    transferControl.dest_IP = in_aton(DST_IP);

    // [L2] Memory allocation for socket buffer
    transferControl.socketBuffer = alloc_skb(transferControl.broadcastLength + NET_IP_ALIGN, GFP_ATOMIC);
    if (!transferControl.socketBuffer)
    {
        return -ENOMEM;
    }

    skb_reserve(transferControl.socketBuffer, NET_IP_ALIGN);
    skb_reserve(transferControl.socketBuffer, ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr));

    // [L7] Application Layer: Write raw payload
    transferControl.Data[DATA_PACKET_UDP] = skb_put(transferControl.socketBuffer, PAYLOAD_LEN);
    memcpy(transferControl.Data[DATA_PACKET_UDP], "Ice.Marek.IceNET.Technology.x-86", PAYLOAD_LEN);

    if (aesEncrypt(transferControl.Data[DATA_PACKET_UDP], PAYLOAD_LEN, (u8 *)aes_key, NULL) < 0)
    {
        pr_err("[TX][UDP][L6] AES encryption failed\n");
        kfree_skb(transferControl.socketBuffer);
        return -EIO;
    }

    print_hex_dump(KERN_INFO, "[TX][UDP][L7] Encrypted Payload: ", DUMP_PREFIX_OFFSET, 16, 1, transferControl.Data[DATA_PACKET_UDP], PAYLOAD_LEN, false);

    // [L4] Transport Layer: UDP header setup
    skb_push(transferControl.socketBuffer, sizeof(struct udphdr));
    transferControl.udpHeader = (struct udphdr *)transferControl.socketBuffer->data;
    transferControl.udpHeader->source = htons(SRC_PORT);
    transferControl.udpHeader->dest = htons(DST_PORT);
    transferControl.udpHeader->len = htons(sizeof(struct udphdr) + PAYLOAD_LEN);
    transferControl.udpHeader->check = 0;

    // [L3] Network Layer: IP header setup
    skb_push(transferControl.socketBuffer, sizeof(struct iphdr));
    transferControl.ipHeader = (struct iphdr *)transferControl.socketBuffer->data;
    transferControl.ipHeader->version = 4;
    transferControl.ipHeader->ihl = 5;
    transferControl.ipHeader->tos = 0;
    transferControl.ipHeader->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + PAYLOAD_LEN);
    transferControl.ipHeader->id = 0;
    transferControl.ipHeader->frag_off = 0;
    transferControl.ipHeader->ttl = 64;
    transferControl.ipHeader->protocol = IPPROTO_UDP;
    transferControl.ipHeader->saddr = transferControl.source_IP;
    transferControl.ipHeader->daddr = transferControl.dest_IP;
    transferControl.ipHeader->check = ip_fast_csum((unsigned char *)transferControl.ipHeader, transferControl.ipHeader->ihl);

    // [L2] Ethernet header setup
    skb_push(transferControl.socketBuffer, ETH_HLEN);
    transferControl.ethernetHeader = (struct ethhdr *)transferControl.socketBuffer->data;
    memcpy(transferControl.ethernetHeader->h_dest, transferControl.destinationMAC, ETH_ALEN);
    memcpy(transferControl.ethernetHeader->h_source, networkControl.networkDevice->dev_addr, ETH_ALEN);
    transferControl.ethernetHeader->h_proto = htons(ETH_P_IP);

    // [L1] Physical Layer (abstracted in kernel): Queue packet for transmission
    transferControl.socketBuffer->dev = networkControl.networkDevice;
    transferControl.socketBuffer->protocol = transferControl.ethernetHeader->h_proto;
    transferControl.socketBuffer->pkt_type = PACKET_BROADCAST;
    transferControl.socketBuffer->ip_summed = CHECKSUM_UNNECESSARY;

    if (dev_queue_xmit(transferControl.socketBuffer) < 0)
    {
        pr_err("[TX][UDP][L1] Failed to transmit UDP packet\n");
        kfree_skb(transferControl.socketBuffer);
        return -EIO;
    }

    pr_info("[TX][UDP][L1] Encrypted UDP packet sent to broadcast\n");

    return 0;
}

int tcpBroadcastTransmission(void)
{

    transferControl.broadcastLength = ETH_HLEN + sizeof(struct iphdr) + sizeof(struct tcphdr) + PAYLOAD_LEN;

    // [L2] Data Link Layer: Set destination to broadcast MAC
    memset(transferControl.destinationMAC, 0xFF, ETH_ALEN);

    // [L3] Network Layer
    transferControl.source_IP = in_aton(SRC_IP);
    transferControl.dest_IP = in_aton(DST_IP);

    transferControl.socketBuffer = alloc_skb(transferControl.broadcastLength + NET_IP_ALIGN, GFP_ATOMIC);
    if (!transferControl.socketBuffer) return -ENOMEM;

    skb_reserve(transferControl.socketBuffer, NET_IP_ALIGN);
    skb_reserve(transferControl.socketBuffer, ETH_HLEN + sizeof(struct iphdr) + sizeof(struct tcphdr));

    // [L7] Application Layer: Payload
    transferControl.Data[DATA_PACKET_TCP] = skb_put(transferControl.socketBuffer, PAYLOAD_LEN);
    memcpy(transferControl.Data[DATA_PACKET_TCP], "Ice.Marek.IceNET.Technology.x-86", PAYLOAD_LEN);

    if (aesEncrypt(transferControl.Data[DATA_PACKET_TCP], PAYLOAD_LEN, (u8 *)aes_key, NULL) < 0)
    {
        pr_err("[TX][TCP][L6] AES encryption failed\n");
        kfree_skb(transferControl.socketBuffer);
        return -EIO;
    }

    print_hex_dump(KERN_INFO, "[TX][TCP][L7] Encrypted Payload: ", DUMP_PREFIX_OFFSET, 16, 1, transferControl.Data[DATA_PACKET_TCP], PAYLOAD_LEN, false);

    // [L4] TCP Header
    skb_push(transferControl.socketBuffer, sizeof(struct tcphdr));
    transferControl.tcpHeader = (struct tcphdr *)transferControl.socketBuffer->data;
    memset(transferControl.tcpHeader, 0, sizeof(struct tcphdr));
    transferControl.tcpHeader->source = htons(44444);
    transferControl.tcpHeader->dest = htons(80);
    transferControl.tcpHeader->seq = htonl(0);
    transferControl.tcpHeader->doff = 5;
    transferControl.tcpHeader->syn = 1;
    transferControl.tcpHeader->window = htons(1024);
    transferControl.tcpHeader->check = 0;

    // [L3] IP Header
    skb_push(transferControl.socketBuffer, sizeof(struct iphdr));
    transferControl.ipHeader = (struct iphdr *)transferControl.socketBuffer->data;
    transferControl.ipHeader->version = 4;
    transferControl.ipHeader->ihl = 5;
    transferControl.ipHeader->ttl = 64;
    transferControl.ipHeader->protocol = IPPROTO_TCP;
    transferControl.ipHeader->saddr = transferControl.source_IP;
    transferControl.ipHeader->daddr = transferControl.dest_IP;
    transferControl.ipHeader->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + PAYLOAD_LEN);
    transferControl.ipHeader->check = ip_fast_csum((unsigned char *)transferControl.ipHeader, transferControl.ipHeader->ihl);

    // [L2] Ethernet Header
    skb_push(transferControl.socketBuffer, ETH_HLEN);
    transferControl.ethernetHeader = (struct ethhdr *)transferControl.socketBuffer->data;
    memcpy(transferControl.ethernetHeader->h_dest, transferControl.destinationMAC, ETH_ALEN);
    memcpy(transferControl.ethernetHeader->h_source, networkControl.networkDevice->dev_addr, ETH_ALEN);
    transferControl.ethernetHeader->h_proto = htons(ETH_P_IP);

    // [L1] Send Packet
    transferControl.socketBuffer->dev = networkControl.networkDevice;
    transferControl.socketBuffer->protocol = transferControl.ethernetHeader->h_proto;
    transferControl.socketBuffer->pkt_type = PACKET_BROADCAST;
    transferControl.socketBuffer->ip_summed = CHECKSUM_UNNECESSARY;

    if (dev_queue_xmit(transferControl.socketBuffer) < 0) {
        pr_err("[TX][TCP][L1] Failed to transmit TCP segment\n");
        kfree_skb(transferControl.socketBuffer);
        return -EIO;
    }

    pr_info("[TX][TCP][L1] Encrypted TCP segment sent\n");
    return 0;
}

// Module Init
int broadcastInit(void)
{
    pr_info("[TX][INIT] Loading IceNET Master Controler\n");

    // [L2] Device Initialization
    networkControl.networkDevice = dev_get_by_name(&init_net, networkControl.iface_name);
    if (!networkControl.networkDevice)
    {
        pr_err("[TX][L2] Device %s not found\n", networkControl.iface_name);
        return -ENODEV;
    }

    udpBroadcastTransmission();
    tcpBroadcastTransmission();

    dev_put(networkControl.networkDevice);

    return 0;
}

// Module Exit
void broadcastDestroy(void)
{
    pr_info("[TX][DESTROY] IceNET Master Controler Module Unloaded\n");
}
