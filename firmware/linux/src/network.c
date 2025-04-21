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
#include <linux/tcp.h>         // L4 - TCP header
#include <linux/skbuff.h>
#include <linux/netdevice.h>   // L2 - Network device structures (Ethernet)
#include <linux/inet.h>
#include <linux/etherdevice.h> // L2 - Ethernet header helpers
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/crypto.h>
#include <linux/scatterlist.h>

#define LISTEN_PORT 54321
#define TEST_PORT 22222
#define BROADCAST_IP 0xC0A808FF // 192.168.8.255

#define AES_BLOCK_SIZE 16

static struct nf_hook_ops netFilterHook;
static struct packet_type arp_packet_type;

typedef struct
{
    struct ethhdr *ethernetHeader;

} hookControlType;

static hookControlType hookControl =
{
    .ethernetHeader = NULL,
};

static int aes_decrypt(void *data, size_t len, u8 *key, u8 *iv)
{
    struct crypto_cipher *tfm;
    int i;

    if (len % AES_BLOCK_SIZE != 0) {
        pr_err("AES decryption error: data size must be multiple of 16 bytes\n");
        return -EINVAL;
    }

    tfm = crypto_alloc_cipher("aes", 0, 0);
    if (IS_ERR(tfm)) {
        pr_err("Failed to allocate AES cipher\n");
        return PTR_ERR(tfm);
    }

    if (crypto_cipher_setkey(tfm, key, 16)) {
        pr_err("Failed to set AES key\n");
        crypto_free_cipher(tfm);
        return -EIO;
    }

    for (i = 0; i < len; i += AES_BLOCK_SIZE) {
        crypto_cipher_decrypt_one(tfm, data + i, data + i);
    }

    crypto_free_cipher(tfm);
    return 0;
}

// ARP packet handler
static int handle_arp_request(struct sk_buff *skb)
{
    struct arphdr *arp;
    unsigned char *arp_ptr;
    unsigned char *sha, *spa, *tha, *tpa;

    arp = arp_hdr(skb);
    if (!arp)
        return NET_RX_DROP;

    if (ntohs(arp->ar_op) != ARPOP_REQUEST)
        return NET_RX_SUCCESS;

    arp_ptr = (unsigned char *)(arp + 1);
    sha = arp_ptr;
    spa = sha + arp->ar_hln;
    tha = spa + arp->ar_pln;
    tpa = tha + arp->ar_hln;

    pr_info("[ARP] ARP request: who has %pI4? Tell %pI4\n", tpa, spa);

    return NET_RX_SUCCESS;
}

static int arp_receive(struct sk_buff *skb, struct net_device *dev,
                       struct packet_type *pt, struct net_device *orig_dev)
{
    if (!skb)
        return NET_RX_DROP;

    skb = skb_share_check(skb, GFP_ATOMIC);
    if (!skb)
        return NET_RX_DROP;

    handle_arp_request(skb);
    kfree_skb(skb);
    return NET_RX_SUCCESS;
}

// Main Netfilter hook
static unsigned int receiverHook(void *priv, struct sk_buff *socketBuffer, const struct nf_hook_state *state)
{
    struct iphdr *iph;
    struct udphdr *udph;
    struct tcphdr *tcph;
    u8 *payload;
    int payload_len;
    unsigned short dport;

    u8 aes_key[16] =
    {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x97, 0x75, 0x46, 0x7e, 0x56, 0x4e
    };

    if (!socketBuffer)
        return NF_ACCEPT;

    if (state->in || state->out)
    {
        pr_info("[L2][ETH] In: %s | Out: %s\n",
                state->in ? state->in->name : "N/A",
                state->out ? state->out->name : "N/A");

        if (state->in && strcmp(state->in->name, "eth0") == 0)
        {
            pr_info("[L2][ETH] Packet received on interface: %s\n", state->in->name);
        }
    }

    if (skb_mac_header_was_set(socketBuffer))
    {
        hookControl.ethernetHeader = eth_hdr(socketBuffer);
        if (hookControl.ethernetHeader)
        {
            pr_info("[L2][ETH] Src MAC: %pM -> Dst MAC: %pM | Proto: 0x%04x\n",
                    hookControl.ethernetHeader->h_source,
                    hookControl.ethernetHeader->h_dest,
                    ntohs(hookControl.ethernetHeader->h_proto));
        }
    }

    iph = ip_hdr(socketBuffer);
    if (!iph)
        return NF_ACCEPT;

    pr_info("[L3][IP] Src IP: %pI4 -> Dst IP: %pI4 | Proto: %d\n",
            &iph->saddr, &iph->daddr, iph->protocol);

    if (iph->protocol == IPPROTO_UDP)
    {
        udph = udp_hdr(socketBuffer);
        if (!udph)
            return NF_ACCEPT;

        dport = ntohs(udph->dest);
        if (dport != LISTEN_PORT && dport != TEST_PORT)
            return NF_ACCEPT;

        if (iph->daddr != htonl(BROADCAST_IP))
            return NF_ACCEPT;

        payload = (u8 *)udph + sizeof(struct udphdr);
        payload_len = ntohs(udph->len) - sizeof(struct udphdr);

        pr_info("[L4][UDP] Src Port: %d -> Dst Port: %d\n", ntohs(udph->source), dport);

        if (payload_len > 0 && skb_tail_pointer(socketBuffer) >= payload + payload_len)
        {
            pr_info("[L7][DATA][UDP] Encrypted Payload: %.32s\n", payload);
            if (aes_decrypt(payload, payload_len, aes_key, NULL) == 0)
            {
                pr_info("[L7][DATA][UDP] Decrypted Payload: %.32s\n", payload);
            }
            else
            {
                pr_err("[L7][DATA][UDP] Decryption failed\n");
            }
        }
        else
        {
            pr_info("[L7][DATA][UDP] Payload not safely accessible\n");
        }
    }
    else if (iph->protocol == IPPROTO_TCP)
    {
        tcph = (struct tcphdr *)((__u32 *)iph + iph->ihl);
        if (!tcph)
            return NF_ACCEPT;

        payload = (u8 *)tcph + (tcph->doff * 4);
        payload_len = ntohs(iph->tot_len) - (iph->ihl * 4) - (tcph->doff * 4);

        pr_info("[L4][TCP] Src Port: %d -> Dst Port: %d\n",
                ntohs(tcph->source), ntohs(tcph->dest));

        if (payload_len > 0 && skb_tail_pointer(socketBuffer) >= payload + payload_len)
        {
            pr_info("[L7][DATA][TCP] Encrypted Payload: %.32s\n", payload);
            if (aes_decrypt(payload, payload_len, aes_key, NULL) == 0)
            {
                pr_info("[L7][DATA][TCP] Decrypted Payload: %.32s\n", payload);
            }
            else
            {
                pr_err("[L7][DATA][TCP] Decryption failed\n");
            }
        }
        else
        {
            pr_info("[L7][DATA][TCP] Payload not safely accessible\n");
        }
    }

    return NF_ACCEPT;
}

// Init routine
void broadcastRxInit(void)
{
    /**
     * [L3] Netfilter hook works only at Layer 3
     * and above meaning IP packets (IPv4, IPv6, etc)
     */
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
        pr_info("[INIT][NET] Sniffing TCP and UDP @ port %d\n", LISTEN_PORT);
    }

    /**
     * [L2] ARP is not IP protocol
     * ARP is Layer 2 protocol instead
     */
    arp_packet_type.type = htons(ETH_P_ARP);
    arp_packet_type.func = arp_receive;
    dev_add_pack(&arp_packet_type);
    pr_info("[INIT][ARP] ARP sniffer registered\n");
}

// Cleanup routine
void broadcastRxDestroy(void)
{
    nf_unregister_net_hook(&init_net, &netFilterHook);
    dev_remove_pack(&arp_packet_type);
    pr_info("[DESTROY][NET] Sniffer stopped\n");
}
