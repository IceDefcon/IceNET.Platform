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

#define LISTEN_PORT 54321      // Must match sender's DST_PORT (L4 - Transport layer)
#define BROADCAST_IP 0xC0A808FF  // 192.168.8.255 in hex (L3 - Network layer)

struct netFilter_hookOperator
{
    nf_hookfn       *hook;
    struct net      *net;
    u_int8_t        pf;
    unsigned int    hooknum;
    int             priority;
};

static struct nf_hook_ops netFilterHook;

// Hook function - executed at L3 level (IP Layer)
static unsigned int receiverHook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph;          // L3 - IP header
    struct udphdr *udph;        // L4 - UDP header
    u8 *udp_payload;

    if (!skb)
        return NF_ACCEPT;

    // Layer 3 - Extract IP header
    iph = ip_hdr(skb);
    if (!iph || iph->protocol != IPPROTO_UDP) // Filter only UDP (L4)
        return NF_ACCEPT;

    // Layer 4 - Extract UDP header
    udph = udp_hdr(skb);
    if (!udph || ntohs(udph->dest) != LISTEN_PORT) // Check destination port (L4)
        return NF_ACCEPT;

    // Layer 3 - Ensure this is a broadcast packet
    if (iph->daddr != htonl(BROADCAST_IP))
        return NF_ACCEPT;

    // Layer 7-ish - Application Data / Payload
    udp_payload = (u8 *)((u8 *)udph + sizeof(struct udphdr));

    // LOG: Received a UDP broadcast packet (L3 + L4 + L7 payload)
    pr_info("[CTRL][NET] Received broadcast From %pI4:%d, payload: %.32s\n", &iph->saddr, ntohs(udph->source), udp_payload);

    return NF_ACCEPT; // Accept packet to continue normal flow
}

// Initialization routine
void broadcastRxInit(void)
{
    // Netfilter hook registration at Layer 3 (IP Layer)
    netFilterHook.hook = receiverHook;
    netFilterHook.hooknum = NF_INET_PRE_ROUTING;   // L3 - Before routing decision
    netFilterHook.pf = PF_INET;                    // L3 - IPv4
    netFilterHook.priority = NF_IP_PRI_FIRST;

    if (nf_register_net_hook(&init_net, &netFilterHook) < 0)
    {
        pr_err("[ERNO][NET] Failed to register netfilter hook\n");
    }

    pr_info("[INIT][NET] Network Listening Broadcast @ UDP port %d\n", LISTEN_PORT);
}

// Cleanup routine
void broadcastRxDestroy(void)
{
    nf_unregister_net_hook(&init_net, &netFilterHook);
    pr_info("[DESTROY][NET] Destroy Broadcast Listener\n");
}


// OSI Layer   Description Where it's used
// L1 - Physical   Actual hardware (cables, NIC)   Not directly handled in this code
// L2 - Data Link (Ethernet)   MAC addressing, Ethernet frames netdevice.h used; but no MAC-level filtering here
// L3 - Network (IP)   IP addresses, routing   ip_hdr(), iph->daddr, IPPROTO_UDP, PF_INET
// L4 - Transport (UDP)    Ports, UDP headers  udp_hdr(), udph->dest, LISTEN_PORT
// L7 - Application    Payload/data    Logging the udp_payload (i.e., message content)
