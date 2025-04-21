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

#define SRC_IP    "192.168.8.101"   // [L3] Source IP
#define DST_IP    "192.168.8.255"   // [L3] tcpBroadcast IP
#define SRC_PORT   12345            // [L4]
#define DST_PORT   54321            // [L4]
#define PAYLOAD_LEN 32              // [L7]
#define AES_KEY_LEN 16              // AES key length for AES-128

// AES encryption key (16 bytes for AES-128)
static const unsigned char aes_key[AES_KEY_LEN] =
{
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x97, 0x75, 0x46, 0x7e, 0x56, 0x4e
};

typedef struct
{
    char *iface_name;
    struct net_device *networkDevice;
    struct sk_buff *socketBuffer;
    struct ethhdr *ethernetHeader;   // [L2]
    struct iphdr *ipHeader;          // [L3]
    struct udphdr *udpHeader;        // [L4]
    unsigned char *data;             // [L7]
    int broadcstLength;
    unsigned char destinationMAC[ETH_ALEN];
    __be32 source_IP;
    __be32 dest_IP;
} TCPbroadcastType;

static TCPbroadcastType tcpBroadcast =
{
    .iface_name = "wlp2s0",
    .networkDevice = NULL,
    .socketBuffer = NULL,
    .ethernetHeader = NULL,
    .ipHeader = NULL,
    .udpHeader = NULL,
    .broadcstLength = 0,
    .destinationMAC = {0},
    .source_IP = 0,
    .dest_IP = 0
};

static int aes_encrypt(void *data, size_t len, u8 *key, u8 *iv)
{
    struct crypto_cipher *tfm;
    int i;

    if (len % AES_KEY_LEN != 0) {
        pr_err("Data length must be a multiple of 16 bytes\n");
        return -EINVAL;
    }

    tfm = crypto_alloc_cipher("aes", 0, 0);
    if (IS_ERR(tfm)) {
        pr_err("Failed to allocate AES cipher\n");
        return PTR_ERR(tfm);
    }

    if (crypto_cipher_setkey(tfm, key, AES_KEY_LEN)) {
        pr_err("Failed to set AES key\n");
        crypto_free_cipher(tfm);
        return -EIO;
    }

    // Encrypt data block-by-block (16 bytes per block)
    for (i = 0; i < len; i += AES_KEY_LEN)
    {
        crypto_cipher_encrypt_one(tfm, data + i, data + i);
    }

    crypto_free_cipher(tfm);
    return 0;
}

int broadcastSendUDP(void)
{
    tcpBroadcast.broadcstLength = ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr) + PAYLOAD_LEN;

    // Set destination MAC address to broadcast
    tcpBroadcast.destinationMAC[0] = 0xFF;
    tcpBroadcast.destinationMAC[1] = 0xFF;
    tcpBroadcast.destinationMAC[2] = 0xFF;
    tcpBroadcast.destinationMAC[3] = 0xFF;
    tcpBroadcast.destinationMAC[4] = 0xFF;
    tcpBroadcast.destinationMAC[5] = 0xFF;

    tcpBroadcast.source_IP = in_aton(SRC_IP);
    tcpBroadcast.dest_IP = in_aton(DST_IP);

    tcpBroadcast.socketBuffer = alloc_skb(tcpBroadcast.broadcstLength + NET_IP_ALIGN, GFP_ATOMIC);
    if (!tcpBroadcast.socketBuffer)
        return -ENOMEM;

    skb_reserve(tcpBroadcast.socketBuffer, NET_IP_ALIGN);
    skb_reserve(tcpBroadcast.socketBuffer, ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr));

    // [L7] Payload - Original message
    tcpBroadcast.data = skb_put(tcpBroadcast.socketBuffer, PAYLOAD_LEN);
    memcpy(tcpBroadcast.data, "Ice.Marek.IceNET.Technology.x-86", PAYLOAD_LEN);

    // AES encrypt the payload
    if (aes_encrypt(tcpBroadcast.data, PAYLOAD_LEN, (u8 *)aes_key, NULL) < 0) {
        pr_err("AES encryption failed\n");
        return -EIO;
    }

    // Replace original payload with encrypted payload
    pr_info("[TX][L7] Encrypted Payload: %.32s\n", tcpBroadcast.data);

    // [L4] UDP Header
    skb_push(tcpBroadcast.socketBuffer, sizeof(struct udphdr));
    tcpBroadcast.udpHeader = (struct udphdr *)tcpBroadcast.socketBuffer->data;
    tcpBroadcast.udpHeader->source = htons(SRC_PORT);
    tcpBroadcast.udpHeader->dest = htons(DST_PORT);
    tcpBroadcast.udpHeader->len = htons(sizeof(struct udphdr) + PAYLOAD_LEN);
    tcpBroadcast.udpHeader->check = 0;

    // [L3] IP Header
    skb_push(tcpBroadcast.socketBuffer, sizeof(struct iphdr));
    tcpBroadcast.ipHeader = (struct iphdr *)tcpBroadcast.socketBuffer->data;
    tcpBroadcast.ipHeader->version = 4;
    tcpBroadcast.ipHeader->ihl = 5;
    tcpBroadcast.ipHeader->tos = 0;
    tcpBroadcast.ipHeader->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + PAYLOAD_LEN);
    tcpBroadcast.ipHeader->id = 0;
    tcpBroadcast.ipHeader->frag_off = 0;
    tcpBroadcast.ipHeader->ttl = 64;
    tcpBroadcast.ipHeader->protocol = IPPROTO_UDP;
    tcpBroadcast.ipHeader->saddr = tcpBroadcast.source_IP;
    tcpBroadcast.ipHeader->daddr = tcpBroadcast.dest_IP;
    tcpBroadcast.ipHeader->check = 0;
    tcpBroadcast.ipHeader->check = ip_fast_csum((unsigned char *)tcpBroadcast.ipHeader, tcpBroadcast.ipHeader->ihl);

    // [L2] Ethernet Header
    skb_push(tcpBroadcast.socketBuffer, ETH_HLEN);
    tcpBroadcast.ethernetHeader = (struct ethhdr *)tcpBroadcast.socketBuffer->data;
    memcpy(tcpBroadcast.ethernetHeader->h_dest, tcpBroadcast.destinationMAC, ETH_ALEN);
    memcpy(tcpBroadcast.ethernetHeader->h_source, tcpBroadcast.networkDevice->dev_addr, ETH_ALEN);
    tcpBroadcast.ethernetHeader->h_proto = htons(ETH_P_IP);

    // [L1] Transmit
    tcpBroadcast.socketBuffer->dev = tcpBroadcast.networkDevice;
    tcpBroadcast.socketBuffer->protocol = tcpBroadcast.ethernetHeader->h_proto;
    tcpBroadcast.socketBuffer->pkt_type = PACKET_BROADCAST;
    tcpBroadcast.socketBuffer->ip_summed = CHECKSUM_UNNECESSARY;

    if (dev_queue_xmit(tcpBroadcast.socketBuffer) < 0)
    {
        pr_err("[TX][L1] Failed to transmit UDP packet\n");
        return -EIO;
    }

    pr_info("[TX][L1] Encrypted UDP packet sent to broadcast\n");
    return 0;
}

int broadcastSendTCP(void)
{
    // AES encrypt the payload
    unsigned char encrypted_payload[PAYLOAD_LEN];

    struct sk_buff *skb;
    struct ethhdr *eth;
    struct iphdr *ip;
    struct tcphdr *tcp;
    unsigned char *data;
    int total_len = ETH_HLEN + sizeof(struct iphdr) + sizeof(struct tcphdr) + PAYLOAD_LEN;
    __be32 src_ip = in_aton(SRC_IP);
    __be32 dst_ip = in_aton(DST_IP);
    unsigned char dst_mac[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    skb = alloc_skb(total_len + NET_IP_ALIGN, GFP_ATOMIC);
    if (!skb) return -ENOMEM;

    skb_reserve(skb, NET_IP_ALIGN);
    skb_reserve(skb, ETH_HLEN + sizeof(struct iphdr) + sizeof(struct tcphdr));

    // [L7] Payload - Original message
    data = skb_put(skb, PAYLOAD_LEN);
    memcpy(data, "Ice.Marek.IceNET.Technology.x-86", PAYLOAD_LEN);

    if (aes_encrypt(data, PAYLOAD_LEN, (u8 *)aes_key, NULL) < 0) {
        pr_err("AES encryption failed\n");
        return -EIO;
    }

    // Replace original payload with encrypted payload
    memcpy(data, encrypted_payload, PAYLOAD_LEN);

    // [L4] TCP Header
    skb_push(skb, sizeof(struct tcphdr));
    tcp = (struct tcphdr *)skb->data;
    memset(tcp, 0, sizeof(struct tcphdr));
    tcp->source = htons(44444);
    tcp->dest = htons(80);
    tcp->seq = htonl(0);
    tcp->doff = 5;
    tcp->syn = 1;
    tcp->window = htons(1024);
    tcp->check = 0; // No checksum for this demo

    // [L3] IP Header
    skb_push(skb, sizeof(struct iphdr));
    ip = (struct iphdr *)skb->data;
    ip->version = 4;
    ip->ihl = 5;
    ip->ttl = 64;
    ip->protocol = IPPROTO_TCP;
    ip->saddr = src_ip;
    ip->daddr = dst_ip;
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + PAYLOAD_LEN);
    ip->check = 0;
    ip->check = ip_fast_csum((unsigned char *)ip, ip->ihl);

    // [L2] Ethernet Header
    skb_push(skb, ETH_HLEN);
    eth = (struct ethhdr *)skb->data;
    memcpy(eth->h_dest, dst_mac, ETH_ALEN);
    memcpy(eth->h_source, tcpBroadcast.networkDevice->dev_addr, ETH_ALEN);
    eth->h_proto = htons(ETH_P_IP);

    // [L1] Transmit
    skb->dev = tcpBroadcast.networkDevice;
    skb->protocol = eth->h_proto;
    skb->pkt_type = PACKET_BROADCAST;
    skb->ip_summed = CHECKSUM_UNNECESSARY;

    if (dev_queue_xmit(skb) < 0)
    {
        pr_err("[TX][TCP][L1] Failed to transmit TCP segment\n");
        return -EIO;
    }

    pr_info("[TX][TCP][L1] Encrypted TCP segment sent\n");
    return 0;
}

// ========== [ Entry Point ] ========== //
int broadcastInit(void)
{
    pr_info("[TX][INIT] Loading IceNET tcpBroadcast Module...\n");

    // [L2] Bind to network interface
    tcpBroadcast.networkDevice = dev_get_by_name(&init_net, tcpBroadcast.iface_name);
    if (!tcpBroadcast.networkDevice)
    {
        pr_err("[TX][L2] Device %s not found\n", tcpBroadcast.iface_name);
        return -ENODEV;
    }

    broadcastSendUDP();         // Function 1
    broadcastSendTCP();         // Function 2

    dev_put(tcpBroadcast.networkDevice);
    return 0;
}

// ========== [ Exit Point ] ========== //
void broadcastDestroy(void)
{
    pr_info("[TX][DESTROY] tcpBroadcast Module Unloaded\n");
}
