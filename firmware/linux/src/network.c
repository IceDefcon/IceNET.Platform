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

static int handle_arp_request(struct sk_buff *skb)
{
    struct arphdr *arp;
    unsigned char *arp_ptr;
    struct sk_buff *reply_skb;
    struct net_device *dev = skb->dev;
    unsigned char *sha, *spa, *tha, *tpa;
    unsigned char *arp_reply_ptr;
    struct ethhdr *eth;
    int arp_hdr_len;

    __be32 sender_ip, target_ip;
    unsigned char our_mac[ETH_ALEN];

    __be32 our_ip = htonl(0xC0A808AE);     // 192.168.8.174
    __be32 allowed_sender_ip = htonl(0xC0A80865);  // 192.168.8.101

    pr_info("[ARP] Received ARP request, skb=%p, dev=%s\n", skb, dev->name);

    arp = arp_hdr(skb);
    if (!arp) {
        pr_err("[ARP] Failed to extract ARP header from skb: %p\n", skb);
        return NET_RX_DROP;
    }
    pr_info("[ARP] Extracted ARP header: arp=%p, operation=%u\n", arp, ntohs(arp->ar_op));

    if (ntohs(arp->ar_op) != ARPOP_REQUEST) {
        pr_info("[ARP] Not an ARP request, skipping\n");
        return NET_RX_SUCCESS;
    }

    arp_ptr = (unsigned char *)(arp + 1);
    sha = arp_ptr;
    spa = sha + arp->ar_hln;
    tha = spa + arp->ar_pln;
    tpa = tha + arp->ar_hln;

    pr_info("[ARP] SHA (Sender MAC) = %pM\n", sha);
    pr_info("[ARP] SPA (Sender IP) = %pI4\n", spa);
    pr_info("[ARP] THA (Target MAC) = %pM\n", tha);
    pr_info("[ARP] TPA (Target IP) = %pI4\n", tpa);

    memcpy(&sender_ip, spa, 4);   // Sender IP
    memcpy(&target_ip, tpa, 4);   // Target IP
    pr_info("[ARP] Sender IP = %pI4, Target IP = %pI4\n", &sender_ip, &target_ip);

    if (target_ip != our_ip || sender_ip != allowed_sender_ip) {
        pr_info("[ARP] Ignoring request from %pI4 for %pI4 (Expected IPs: sender=%pI4, target=%pI4)\n",
                &sender_ip, &target_ip, &allowed_sender_ip, &our_ip);
        return NET_RX_SUCCESS;
    }

    pr_info("[ARP] Responding to ARP request from %pI4 asking for %pI4\n", &sender_ip, &target_ip);

    memcpy(our_mac, dev->dev_addr, ETH_ALEN); // Our MAC
    pr_info("[ARP] Our MAC address: %pM\n", our_mac);

    arp_hdr_len = sizeof(struct arphdr) + 2 * (ETH_ALEN + 4); // hln=6, pln=4
    pr_info("[ARP] ARP header length calculated: %d\n", arp_hdr_len);

    reply_skb = alloc_skb(ETH_HLEN + arp_hdr_len, GFP_ATOMIC);
    if (!reply_skb) {
        pr_err("[ARP] Failed to allocate skb for ARP reply\n");
        return NET_RX_DROP;
    }
    pr_info("[ARP] Successfully allocated skb for ARP reply: reply_skb=%p\n", reply_skb);

    skb_reserve(reply_skb, ETH_HLEN);               // Reserve space for Ethernet header
    skb_put(reply_skb, arp_hdr_len);                // Make room for ARP header/data
    eth = (struct ethhdr *)skb_push(reply_skb, ETH_HLEN);            // Push Ethernet header

    pr_info("[ARP] skb headroom: %d, tailroom: %d\n",
            skb_headroom(reply_skb), skb_tailroom(reply_skb));

    reply_skb->dev = dev;
    reply_skb->protocol = htons(ETH_P_ARP);
    skb_reset_mac_header(reply_skb);
    skb_reset_network_header(reply_skb);

    pr_info("[ARP] MAC header set at: %p\n", skb_mac_header(reply_skb));

    eth = (struct ethhdr *)skb_mac_header(reply_skb);
    memcpy(eth->h_dest, sha, ETH_ALEN);         // Dest: sender MAC
    memcpy(eth->h_source, our_mac, ETH_ALEN);   // Src: our MAC
    eth->h_proto = htons(ETH_P_ARP);            // Protocol type

    pr_info("[ARP] Ethernet header: dst=%pM, src=%pM, proto=0x%04x\n",
            eth->h_dest, eth->h_source, ntohs(eth->h_proto));

    // Alignment check (paranoid safety for non-x86 archs)
    BUILD_BUG_ON(__alignof__(struct arphdr) > __alignof__(unsigned long));

    // Setup ARP response
    arp = (struct arphdr *)(eth + 1);
    memset(arp, 0, sizeof(struct arphdr)); // Zero for safety

    arp->ar_hrd = htons(ARPHRD_ETHER);
    arp->ar_pro = htons(ETH_P_IP);
    arp->ar_hln = ETH_ALEN;
    arp->ar_pln = 4;
    arp->ar_op  = htons(ARPOP_REPLY);

    pr_info("[ARP] ARP header: hrd=%u, pro=%u, hln=%d, pln=%d, op=%u\n",
            ntohs(arp->ar_hrd), ntohs(arp->ar_pro),
            arp->ar_hln, arp->ar_pln, ntohs(arp->ar_op));

    // Fill ARP payload
    arp_reply_ptr = (unsigned char *)(arp + 1);
    memcpy(arp_reply_ptr, our_mac, ETH_ALEN);                // Sender MAC
    memcpy(arp_reply_ptr + ETH_ALEN, &our_ip, 4);            // Sender IP
    memcpy(arp_reply_ptr + ETH_ALEN + 4, sha, ETH_ALEN);     // Target MAC
    memcpy(arp_reply_ptr + 2 * ETH_ALEN + 4, spa, 4);        // Target IP

    pr_info("[ARP] ARP payload: sender MAC=%pM, sender IP=%pI4, target MAC=%pM, target IP=%pI4\n",
            our_mac, &our_ip, sha, &sender_ip);

    pr_info("[ARP] Sending ARP reply with dev_queue_xmit (raw Ethernet frame)\n");
    dev_queue_xmit(reply_skb);

    pr_info("[ARP] ARP reply successfully sent to %pI4\n", &sender_ip);
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

void networkInit(void)
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

void networkDestroy(void)
{
    nf_unregister_net_hook(&init_net, &netFilterHook);
    dev_remove_pack(&arp_packet_type);
    pr_info("[DESTROY][NET] Sniffer stopped\n");
}
