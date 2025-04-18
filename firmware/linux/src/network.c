/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
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

struct netFilter_hookOperator
{
    nf_hookfn       *hook;        // Pointer to your hook function
    struct net      *net;         // Network namespace (can be NULL)
    u_int8_t        pf;           // Protocol family -> PF_INET (IPv4), PF_INET6 (IPv6).
    unsigned int    hooknum;      // Hook point (e.g., NF_INET_PRE_ROUTING)
    int             priority;     // Hook priority (lower runs first)
};

static struct nf_hook_ops netFilterHook;

static unsigned int receiverHook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
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
    pr_info("[CTRL][NET] Received broadcast From %pI4:%d, payload: %.32s\n", &iph->saddr, ntohs(udph->source), udp_payload);

    return NF_ACCEPT;
}

void broadcastRxInit(void)
{
    netFilterHook.hook = receiverHook;
    netFilterHook.hooknum = NF_INET_PRE_ROUTING;
    netFilterHook.pf = PF_INET;
    netFilterHook.priority = NF_IP_PRI_FIRST;

    if (nf_register_net_hook(&init_net, &netFilterHook) < 0)
    {
        pr_err("[ERNO][NET] Failed to register netfilter hook\n");
    }
    pr_info("[INIT][NET] Network Listening Broadcast @ UDP port %d\n", LISTEN_PORT);
}

void broadcastRxDestroy(void)
{
    nf_unregister_net_hook(&init_net, &netFilterHook);
    pr_info("[DESTROY][NET] Destroy Broadcast Listener\n");
}
