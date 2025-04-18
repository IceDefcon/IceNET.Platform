/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/etherdevice.h>
#include <linux/inet.h>  // for in4_pton

#define SRC_IP     "192.168.8.101"   // Replace with your actual IP
#define DST_IP     "192.168.8.255"   // Broadcast address
#define SRC_PORT   12345
#define DST_PORT   54321
#define PAYLOAD_LEN 32

typedef struct
{
    char *iface_name;
    struct net_device *dev;
}broadcastType;

static broadcastType Broadcast =
{
    .iface_name = "wlp2s0",
};

int broadcastInit(void)
{
    struct sk_buff *skb;
    struct ethhdr *eth;
    struct iphdr *iph;
    struct udphdr *udph;
    unsigned char *data;
    int total_len = ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr) + PAYLOAD_LEN;
    unsigned char dst_mac[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; // Broadcast MAC
    __be32 src_ip = 0, dst_ip = 0;

    pr_info("[TX] Loading module...\n");

    if (!in4_pton(SRC_IP, -1, (u8 *)&src_ip, -1, NULL) ||
        !in4_pton(DST_IP, -1, (u8 *)&dst_ip, -1, NULL)) {
        pr_err("[TX] Invalid IP format\n");
        return -EINVAL;
    }

    Broadcast.dev = dev_get_by_name(&init_net, Broadcast.iface_name);
    if (!Broadcast.dev) {
        pr_err("[TX] Device %s not found\n", Broadcast.iface_name);
        return -ENODEV;
    }

    skb = alloc_skb(total_len + NET_IP_ALIGN, GFP_ATOMIC);
    if (!skb) {
        dev_put(Broadcast.dev);
        pr_err("[TX] Failed to allocate skb\n");
        return -ENOMEM;
    }

    skb_reserve(skb, NET_IP_ALIGN);
    skb_reserve(skb, ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr));
    data = skb_put(skb, PAYLOAD_LEN);

    memset(data, 0, PAYLOAD_LEN);  // Dummy payload

    data[0] = 0x49;
    data[1] = 0x63;
    data[2] = 0x65;
    data[3] = 0x2E;
    data[4] = 0x4D;
    data[5] = 0x61;
    data[6] = 0x72;
    data[7] = 0x65;
    data[8] = 0x6B;
    data[9] = 0x2E;
    data[10] = 0x49;
    data[11] = 0x63;
    data[12] = 0x65;
    data[13] = 0x4E;
    data[14] = 0x45;
    data[15] = 0x54;
    data[16] = 0x2E;
    data[17] = 0x54;
    data[18] = 0x65;
    data[19] = 0x63;
    data[20] = 0x68;
    data[21] = 0x6E;
    data[22] = 0x6F;
    data[23] = 0x6C;
    data[24] = 0x6F;
    data[25] = 0x67;
    data[26] = 0x79;
    data[27] = 0x2E;
    data[28] = 0x78;
    data[29] = 0x2D;
    data[30] = 0x38;
    data[31] = 0x36;

    skb_push(skb, sizeof(struct udphdr));
    udph = (struct udphdr *)skb->data;
    udph->source = htons(SRC_PORT);
    udph->dest = htons(DST_PORT);
    udph->len = htons(sizeof(struct udphdr) + PAYLOAD_LEN);
    udph->check = 0;

    skb_push(skb, sizeof(struct iphdr));
    iph = (struct iphdr *)skb->data;
    iph->version = 4;
    iph->ihl = 5;
    iph->tos = 0;
    iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + PAYLOAD_LEN);
    iph->id = 0;
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_UDP;
    iph->saddr = src_ip;
    iph->daddr = dst_ip;
    iph->check = 0;
    iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);

    skb_push(skb, ETH_HLEN);
    eth = (struct ethhdr *)skb->data;
    memcpy(eth->h_dest, dst_mac, ETH_ALEN);
    memcpy(eth->h_source, Broadcast.dev->dev_addr, ETH_ALEN);
    eth->h_proto = htons(ETH_P_IP);

    skb->dev = Broadcast.dev;
    skb->protocol = eth->h_proto;
    skb->pkt_type = PACKET_BROADCAST;
    skb->ip_summed = CHECKSUM_UNNECESSARY;

    if (dev_queue_xmit(skb) < 0)
    {
        pr_err("[TX] broadcast_kmod: Failed to transmit packet\n");
        dev_put(Broadcast.dev);
        return -EIO;
    }

    pr_info("[TX] Packet sent via %s to %s\n", Broadcast.iface_name, DST_IP);
    dev_put(Broadcast.dev);
    return 0;
}

void broadcastDestroy(void)
{
    pr_info("[TX] Module unloaded\n");
}

