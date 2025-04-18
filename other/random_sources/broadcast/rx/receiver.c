#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/inet.h>

#define LISTEN_PORT 54321  // Must match sender's DST_PORT
#define BROADCAST_IP 0xC0A808FF  // 192.168.8.255 in hex

static struct nf_hook_ops nfho;

static unsigned int recv_hook(void *priv,
                              struct sk_buff *skb,
                              const struct nf_hook_state *state)
{
    struct iphdr *iph;
    struct udphdr *udph;
    u8 *udp_payload;

    if (!skb)
        return NF_ACCEPT;

    iph = ip_hdr(skb);
    if (!iph || iph->protocol != IPPROTO_UDP)
        return NF_ACCEPT;

    udph = udp_hdr(skb);
    if (!udph || ntohs(udph->dest) != LISTEN_PORT)
        return NF_ACCEPT;

    if (iph->daddr != htonl(BROADCAST_IP))
        return NF_ACCEPT;

    udp_payload = (u8 *)((u8 *)udph + sizeof(struct udphdr));
    pr_info("[RX] Received broadcast UDP from %pI4:%d, payload: %.16s\n",
            &iph->saddr, ntohs(udph->source), udp_payload);

    return NF_ACCEPT;
}

static int __init recv_init(void)
{
    nfho.hook = recv_hook;
    nfho.hooknum = NF_INET_PRE_ROUTING;
    nfho.pf = PF_INET;
    nfho.priority = NF_IP_PRI_FIRST;

    if (nf_register_net_hook(&init_net, &nfho) < 0) {
        pr_err("[RX] Failed to register netfilter hook\n");
        return -1;
    }

    pr_info("[RX] Module loaded, listening on UDP port %d\n", LISTEN_PORT);
    return 0;
}

static void __exit recv_exit(void)
{
    nf_unregister_net_hook(&init_net, &nfho);
    pr_info("[RX] Module unloaded\n");
}

module_init(recv_init);
module_exit(recv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT + You");
MODULE_DESCRIPTION("Kernel module to receive UDP broadcast packets");
