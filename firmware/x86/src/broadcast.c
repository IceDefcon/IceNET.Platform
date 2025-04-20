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
#include <linux/etherdevice.h>
#include <linux/inet.h>

#define SRC_IP     "192.168.8.101"   // [L3] Source IP
#define DST_IP     "192.168.8.255"   // [L3] Broadcast IP
#define SRC_PORT   12345             // [L4]
#define DST_PORT   54321             // [L4]
#define PAYLOAD_LEN 32               // [L7]

typedef struct
{
    char *iface_name;
    struct net_device *networkDevice;
    struct sk_buff *socketBuffer;
    struct ethhdr *ethernetHeader;       // [L2]
    struct iphdr *ipHeader;        // [L3]
    struct udphdr *udpHeader;      // [L4]
    unsigned char *data;      // [L7]
} broadcastType;

static broadcastType Broadcast =
{
    .iface_name = "wlp2s0",
    .networkDevice = NULL,
    .socketBuffer = NULL,
    .ethernetHeader = NULL,
    .ipHeader = NULL,
    .udpHeader = NULL,
};

int broadcastInit(void)
{
    int total_len = ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr) + PAYLOAD_LEN;
    unsigned char dst_mac[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; // [L2] Broadcast MAC
    __be32 src_ip = 0, dst_ip = 0;

    pr_info("[TX][INIT] Loading Broadcast Module...\n");

    // [L3] Convert IP strings to binary
    if (!in4_pton(SRC_IP, -1, (u8 *)&src_ip, -1, NULL) || !in4_pton(DST_IP, -1, (u8 *)&dst_ip, -1, NULL))
    {
        pr_err("[TX][L3] Invalid IP format\n");
        return -EINVAL;
    }

    // [L2] Get network interface
    Broadcast.networkDevice = dev_get_by_name(&init_net, Broadcast.iface_name);
    if (!Broadcast.networkDevice)
    {
        pr_err("[TX][L2] Device %s not found\n", Broadcast.iface_name);
        return -ENODEV;
    }

    // Allocate and prepare socketBuffer
    Broadcast.socketBuffer = alloc_skb(total_len + NET_IP_ALIGN, GFP_ATOMIC);
    if (!Broadcast.socketBuffer)
    {
        dev_put(Broadcast.networkDevice);
        pr_err("[TX][MEM] Failed to allocate socketBuffer\n");
        return -ENOMEM;
    }

    skb_reserve(Broadcast.socketBuffer, NET_IP_ALIGN);
    skb_reserve(Broadcast.socketBuffer, ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr));

    // [L7] Fill payload
    Broadcast.data = skb_put(Broadcast.socketBuffer, PAYLOAD_LEN);
    memset(Broadcast.data, 0, PAYLOAD_LEN);
    memcpy(Broadcast.data, "Ice.Marek.IceNET.Technology.x-86", PAYLOAD_LEN);
    pr_info("[TX][L7] Payload prepared: %.32s\n", Broadcast.data);

    // [L4] Build UDP header
    skb_push(Broadcast.socketBuffer, sizeof(struct udphdr));
    Broadcast.udpHeader = (struct udphdr *)Broadcast.socketBuffer->data;
    Broadcast.udpHeader->source = htons(SRC_PORT);
    Broadcast.udpHeader->dest = htons(DST_PORT);
    Broadcast.udpHeader->len = htons(sizeof(struct udphdr) + PAYLOAD_LEN);
    Broadcast.udpHeader->check = 0;
    pr_info("[TX][L4] UDP SrcPort=%d -> DstPort=%d\n", SRC_PORT, DST_PORT);

    // [L3] Build IP header
    skb_push(Broadcast.socketBuffer, sizeof(struct iphdr));
    Broadcast.ipHeader = (struct iphdr *)Broadcast.socketBuffer->data;
    Broadcast.ipHeader->version = 4;
    Broadcast.ipHeader->ihl = 5;
    Broadcast.ipHeader->tos = 0;
    Broadcast.ipHeader->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + PAYLOAD_LEN);
    Broadcast.ipHeader->id = 0;
    Broadcast.ipHeader->frag_off = 0;
    Broadcast.ipHeader->ttl = 64;
    Broadcast.ipHeader->protocol = IPPROTO_UDP;
    Broadcast.ipHeader->saddr = src_ip;
    Broadcast.ipHeader->daddr = dst_ip;
    Broadcast.ipHeader->check = 0;
    Broadcast.ipHeader->check = ip_fast_csum((unsigned char *)Broadcast.ipHeader, Broadcast.ipHeader->ihl);
    pr_info("[TX][L3] IP Src=%pI4 -> Dst=%pI4\n", &Broadcast.ipHeader->saddr, &Broadcast.ipHeader->daddr);

    // [L2] Build Ethernet header
    skb_push(Broadcast.socketBuffer, ETH_HLEN);
    Broadcast.ethernetHeader = (struct ethhdr *)Broadcast.socketBuffer->data;
    memcpy(Broadcast.ethernetHeader->h_dest, dst_mac, ETH_ALEN);
    memcpy(Broadcast.ethernetHeader->h_source, Broadcast.networkDevice->dev_addr, ETH_ALEN);
    Broadcast.ethernetHeader->h_proto = htons(ETH_P_IP);
    pr_info("[TX][L2] Ethernet SrcMAC=%pM -> DstMAC=%pM\n", Broadcast.ethernetHeader->h_source, Broadcast.ethernetHeader->h_dest);

    // [L1/L2] Send packet
    Broadcast.socketBuffer->dev = Broadcast.networkDevice;
    Broadcast.socketBuffer->protocol = Broadcast.ethernetHeader->h_proto;
    Broadcast.socketBuffer->pkt_type = PACKET_BROADCAST;
    Broadcast.socketBuffer->ip_summed = CHECKSUM_UNNECESSARY;

    if (dev_queue_xmit(Broadcast.socketBuffer) < 0)
    {
        pr_err("[TX][L1] Failed to transmit packet\n");
        dev_put(Broadcast.networkDevice);
        return -EIO;
    }

    pr_info("[TX][L1] Packet sent on interface %s to broadcast IP %s\n", Broadcast.iface_name, DST_IP);
    dev_put(Broadcast.networkDevice);
    return 0;
}

void broadcastDestroy(void)
{
    pr_info("[TX][DESTROY] Module unloaded\n");
}
