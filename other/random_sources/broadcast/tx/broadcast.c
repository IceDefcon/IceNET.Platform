#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
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

static char *iface_name = "wlp2s0";  // Wi-Fi interface
module_param(iface_name, charp, 0000);
MODULE_PARM_DESC(iface_name, "Network interface name");

static int __init broadcast_init(void)
{
    struct net_device *dev;
    struct sk_buff *skb;
    struct ethhdr *eth;
    struct iphdr *iph;
    struct udphdr *udph;
    unsigned char *data;
    int total_len = ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr) + PAYLOAD_LEN;
    unsigned char dst_mac[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; // Broadcast MAC
    __be32 src_ip = 0, dst_ip = 0;

    pr_info("broadcast_kmod: Loading module...\n");

    if (!in4_pton(SRC_IP, -1, (u8 *)&src_ip, -1, NULL) ||
        !in4_pton(DST_IP, -1, (u8 *)&dst_ip, -1, NULL)) {
        pr_err("broadcast_kmod: Invalid IP format\n");
        return -EINVAL;
    }

    dev = dev_get_by_name(&init_net, iface_name);
    if (!dev) {
        pr_err("broadcast_kmod: Device %s not found\n", iface_name);
        return -ENODEV;
    }

    skb = alloc_skb(total_len + NET_IP_ALIGN, GFP_ATOMIC);
    if (!skb) {
        dev_put(dev);
        pr_err("broadcast_kmod: Failed to allocate skb\n");
        return -ENOMEM;
    }

    skb_reserve(skb, NET_IP_ALIGN);
    skb_reserve(skb, ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr));
    data = skb_put(skb, PAYLOAD_LEN);
    memset(data, 'X', PAYLOAD_LEN);  // Dummy payload

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
    memcpy(eth->h_source, dev->dev_addr, ETH_ALEN);
    eth->h_proto = htons(ETH_P_IP);

    skb->dev = dev;
    skb->protocol = eth->h_proto;
    skb->pkt_type = PACKET_BROADCAST;
    skb->ip_summed = CHECKSUM_UNNECESSARY;

    if (dev_queue_xmit(skb) < 0) {
        pr_err("broadcast_kmod: Failed to transmit packet\n");
        dev_put(dev);
        return -EIO;
    }

    pr_info("broadcast_kmod: Packet sent via %s to %s\n", iface_name, DST_IP);
    dev_put(dev);
    return 0;
}

static void __exit broadcast_exit(void)
{
    pr_info("broadcast_kmod: Module unloaded\n");
}

module_init(broadcast_init);
module_exit(broadcast_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT + You");
MODULE_DESCRIPTION("Kernel module to send broadcast UDP packet over Wi-Fi");
