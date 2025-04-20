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
#include <linux/ip.h>          // L3 - IP header
#include <linux/udp.h>         // L4 - UDP header
#include <linux/skbuff.h>
#include <linux/netdevice.h>   // L2 - Network device structures (Ethernet)
#include <linux/inet.h>
#include <linux/etherdevice.h> // L2 - Ethernet header helpers

#define LISTEN_PORT 54321
#define BROADCAST_IP 0xC0A808FF // 192.168.8.255

static struct nf_hook_ops netFilterHook;

// Hook function - processes L2, L3, and L4
static unsigned int receiverHook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct ethhdr *eth;         // L2 - Ethernet header
    struct iphdr *iph;          // L3 - IP header
    struct udphdr *udph;        // L4 - UDP header
    u8 *udp_payload;

    if (!skb)
        return NF_ACCEPT;

    // L2 - Extract Ethernet header
    if (skb_mac_header_was_set(skb))
    {
        eth = eth_hdr(skb);
        if (eth)
        {
            pr_info("[L2][ETH] Src MAC: %pM -> Dst MAC: %pM | Proto: 0x%04x\n", eth->h_source, eth->h_dest, ntohs(eth->h_proto));
        }
    }

    // L3 - Extract IP header
    iph = ip_hdr(skb);
    if (!iph || iph->protocol != IPPROTO_UDP)
    {
        pr_info("[L3][IP] Extract IP header\n");
        return NF_ACCEP
    }

    // L4 - Extract UDP header
    udph = udp_hdr(skb);
    if (!udph || ntohs(udph->dest) != LISTEN_PORT)
    {
        pr_info("[L4][UDP] Extract UDP header\n");
        return NF_ACCEPT;
    }

    // L3 - Check for broadcast IP
    if (iph->daddr != htonl(BROADCAST_IP))
    {
        pr_info("[L3][IP] Check for broadcast IP\n");
        return NF_ACCEPT;
    }

    // L7-ish - Extract payload
    udp_payload = (u8 *)((u8 *)udph + sizeof(struct udphdr));

    // Final log including L3 + L4 + L7
    pr_info("[L3][IP] Src IP: %pI4 -> Dst IP: %pI4\n", &iph->saddr, &iph->daddr);
    pr_info("[L4][UDP] Src Port: %d -> Dst Port: %d\n", ntohs(udph->source), ntohs(udph->dest));
    pr_info("[L7][DATA] Payload: %.32s\n", udp_payload);

    return NF_ACCEPT;
}

// Init routine
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
    else
    {
        pr_info("[INIT][NET] Listening for UDP Broadcast @ port %d\n", LISTEN_PORT);
    }
}

// Cleanup routine
void broadcastRxDestroy(void)
{
    nf_unregister_net_hook(&init_net, &netFilterHook);
    pr_info("[DESTROY][NET] Broadcast Listener Stopped\n");
}
