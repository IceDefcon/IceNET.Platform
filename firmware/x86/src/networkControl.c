/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "networkControl.h"
#include "crypto.h"

#define TARGET_IP    "192.168.8.174"
#define SRC_IP       "192.168.8.101"
#define DST_IP       "192.168.8.255"
#define SRC_PORT     12345
#define DST_PORT     54321
#define PAYLOAD_LEN  32

#define IFACE_NAME  "wlp2s0"
#define LISTEN_PORT 54321
#define TEST_PORT 22222
#define BROADCAST_IP 0xC0A808FF // 192.168.8.255

// [L7] Application Layer: Static payload encryption key
static const unsigned char aes_key[AES_KEY_LEN] =
{
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x97, 0x75, 0x46, 0x7e, 0x56, 0x4e
};

static struct packet_type arp_packet_type;

static networkControlType networkControl =
{
    .networkDevice = NULL,
    .iface_name = IFACE_NAME,
};

static transferControlType transferControl =
{
    .socketBuffer = NULL,
    .ethernetHeader = NULL,
    .ipHeader = NULL,
    .udpHeader = NULL,
    .tcpHeader = NULL,
    .Data =
    {
        [DATA_PACKET_UDP] = NULL,
        [DATA_PACKET_TCP] = NULL,
    },
    .transmissionLength = 0,
    .destinationMAC = {0},
    .source_IP = 0,
    .dest_IP = 0
};

static struct nf_hook_ops netFilterHook;

typedef struct
{
    struct ethhdr *ethernetHeader;

} hookControlType;

static hookControlType hookControl =
{
    .ethernetHeader = NULL,
};

static int arpResponse(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev)
{
    struct ethhdr *eth;
    struct arpHeader *arp;

    if (!skb) {
        pr_err("[RX][ARP] Debug 0: skb is NULL\n");
        return NET_RX_DROP;
    }

    eth = eth_hdr(skb);
    if (!eth)
    {
        pr_err("[RX][ARP] Debug 1: Failed to get Ethernet header\n");
        return NET_RX_DROP;
    }

    if (ntohs(eth->h_proto) != ETH_P_ARP)
    {
        pr_err("[RX][ARP] Debug 2: Not an ARP packet, proto=0x%04x\n", ntohs(eth->h_proto));
        return NET_RX_DROP;
    }

    if (skb->len < (ETH_HLEN + sizeof(struct arpHeader)))
    {
        pr_err("[RX][ARP] Debug 3: Packet too short, len=%u\n", skb->len);
        return NET_RX_DROP;
    }

    arp = (struct arpHeader *)skb_network_header(skb);
    if (!arp)
    {
        pr_err("[RX][ARP] Debug 4: Failed to parse ARP header from skb\n");
        return NET_RX_DROP;
    }

    pr_info("[RX][ARP] Raw ar_op = 0x%04x (%u)\n", ntohs(arp->ar_op), ntohs(arp->ar_op));

    if (ntohs(arp->ar_op) != ARPOP_REPLY)
    {
        pr_err("[RX][ARP] Debug 5: Not an ARP reply, op=%u\n", ntohs(arp->ar_op));
        print_hex_dump(KERN_INFO, "[RX][ARP] Payload: ", DUMP_PREFIX_OFFSET, 16, 1, skb_network_header(skb), skb->len - ETH_HLEN, false);
        return NET_RX_DROP;
    }

    if (arp->ar_sip != in_aton(TARGET_IP))
    {
        pr_err("[RX][ARP] Debug 6: Reply not from expected IP (%pI4), got %pI4\n", &TARGET_IP, &arp->ar_sip);
        return NET_RX_DROP;
    }

    pr_info("[RX][ARP] Got reply from %pI4 with MAC %pM\n", &arp->ar_sip, arp->ar_sha);

    return NET_RX_SUCCESS;
}

int udpTransmission(void)
{
    transferControl.transmissionLength = ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr) + PAYLOAD_LEN;

    // [L2] Data Link Layer: Set destination to broadcast MAC
    memset(transferControl.destinationMAC, 0xFF, ETH_ALEN);

    // [L3] Network Layer
    transferControl.source_IP = in_aton(SRC_IP);
    transferControl.dest_IP = in_aton(DST_IP);

    // [L2] Memory allocation for socket buffer
    transferControl.socketBuffer = alloc_skb(transferControl.transmissionLength + NET_IP_ALIGN, GFP_ATOMIC);
    if (!transferControl.socketBuffer)
    {
        return -ENOMEM;
    }

    skb_reserve(transferControl.socketBuffer, NET_IP_ALIGN);
    skb_reserve(transferControl.socketBuffer, ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr));

    // [L7] Application Layer: Write raw payload
    transferControl.Data[DATA_PACKET_UDP] = skb_put(transferControl.socketBuffer, PAYLOAD_LEN);
    memcpy(transferControl.Data[DATA_PACKET_UDP], "Ice.Marek.IceNET.Technology.x-86", PAYLOAD_LEN);

    if (aesEncrypt(transferControl.Data[DATA_PACKET_UDP], PAYLOAD_LEN, (u8 *)aes_key, NULL) < 0)
    {
        pr_err("[TX][UDP][L6] AES encryption failed\n");
        kfree_skb(transferControl.socketBuffer);
        return -EIO;
    }

    print_hex_dump(KERN_INFO, "[TX][UDP][L7] Encrypted Payload: ", DUMP_PREFIX_OFFSET, 16, 1, transferControl.Data[DATA_PACKET_UDP], PAYLOAD_LEN, false);

    // [L4] Transport Layer: UDP header setup
    skb_push(transferControl.socketBuffer, sizeof(struct udphdr));
    transferControl.udpHeader = (struct udphdr *)transferControl.socketBuffer->data;
    transferControl.udpHeader->source = htons(SRC_PORT);
    transferControl.udpHeader->dest = htons(DST_PORT);
    transferControl.udpHeader->len = htons(sizeof(struct udphdr) + PAYLOAD_LEN);
    transferControl.udpHeader->check = 0;

    // [L3] Network Layer: IP header setup
    skb_push(transferControl.socketBuffer, sizeof(struct iphdr));
    transferControl.ipHeader = (struct iphdr *)transferControl.socketBuffer->data;
    transferControl.ipHeader->version = 4;
    transferControl.ipHeader->ihl = 5;
    transferControl.ipHeader->tos = 0;
    transferControl.ipHeader->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + PAYLOAD_LEN);
    transferControl.ipHeader->id = 0;
    transferControl.ipHeader->frag_off = 0;
    transferControl.ipHeader->ttl = 64;
    transferControl.ipHeader->protocol = IPPROTO_UDP;
    transferControl.ipHeader->saddr = transferControl.source_IP;
    transferControl.ipHeader->daddr = transferControl.dest_IP;
    transferControl.ipHeader->check = ip_fast_csum((unsigned char *)transferControl.ipHeader, transferControl.ipHeader->ihl);

    // [L2] Ethernet header setup
    skb_push(transferControl.socketBuffer, ETH_HLEN);
    transferControl.ethernetHeader = (struct ethhdr *)transferControl.socketBuffer->data;
    memcpy(transferControl.ethernetHeader->h_dest, transferControl.destinationMAC, ETH_ALEN);
    memcpy(transferControl.ethernetHeader->h_source, networkControl.networkDevice->dev_addr, ETH_ALEN);
    transferControl.ethernetHeader->h_proto = htons(ETH_P_IP);

    // [L1] Physical Layer (abstracted in kernel): Queue packet for transmission
    transferControl.socketBuffer->dev = networkControl.networkDevice;
    transferControl.socketBuffer->protocol = transferControl.ethernetHeader->h_proto;
    transferControl.socketBuffer->pkt_type = PACKET_BROADCAST;
    transferControl.socketBuffer->ip_summed = CHECKSUM_UNNECESSARY;

    if (dev_queue_xmit(transferControl.socketBuffer) < 0)
    {
        pr_err("[TX][UDP][L1] Failed to transmit UDP packet\n");
        kfree_skb(transferControl.socketBuffer);
        return -EIO;
    }

    pr_info("[TX][UDP][L1] Encrypted UDP packet sent to broadcast\n");

    return 0;
}

int tcpTransmission(void)
{
    transferControl.transmissionLength = ETH_HLEN + sizeof(struct iphdr) + sizeof(struct tcphdr) + PAYLOAD_LEN;

    // [L2] Data Link Layer: Set destination to broadcast MAC
    memset(transferControl.destinationMAC, 0xFF, ETH_ALEN);

    // [L3] Network Layer
    transferControl.source_IP = in_aton(SRC_IP);
    transferControl.dest_IP = in_aton(DST_IP);

    transferControl.socketBuffer = alloc_skb(transferControl.transmissionLength + NET_IP_ALIGN, GFP_ATOMIC);
    if (!transferControl.socketBuffer) return -ENOMEM;

    skb_reserve(transferControl.socketBuffer, NET_IP_ALIGN);
    skb_reserve(transferControl.socketBuffer, ETH_HLEN + sizeof(struct iphdr) + sizeof(struct tcphdr));

    // [L7] Application Layer: Payload
    transferControl.Data[DATA_PACKET_TCP] = skb_put(transferControl.socketBuffer, PAYLOAD_LEN);
    memcpy(transferControl.Data[DATA_PACKET_TCP], "Ice.Marek.IceNET.Technology.x-86", PAYLOAD_LEN);

    if (aesEncrypt(transferControl.Data[DATA_PACKET_TCP], PAYLOAD_LEN, (u8 *)aes_key, NULL) < 0)
    {
        pr_err("[TX][TCP][L6] AES encryption failed\n");
        kfree_skb(transferControl.socketBuffer);
        return -EIO;
    }

    print_hex_dump(KERN_INFO, "[TX][TCP][L7] Encrypted Payload: ", DUMP_PREFIX_OFFSET, 16, 1, transferControl.Data[DATA_PACKET_TCP], PAYLOAD_LEN, false);

    // [L4] TCP Header
    skb_push(transferControl.socketBuffer, sizeof(struct tcphdr));
    transferControl.tcpHeader = (struct tcphdr *)transferControl.socketBuffer->data;
    memset(transferControl.tcpHeader, 0, sizeof(struct tcphdr));
    transferControl.tcpHeader->source = htons(44444);
    transferControl.tcpHeader->dest = htons(80);
    transferControl.tcpHeader->seq = htonl(0);
    transferControl.tcpHeader->doff = 5;
    transferControl.tcpHeader->syn = 1;
    transferControl.tcpHeader->window = htons(1024);
    transferControl.tcpHeader->check = 0;

    // [L3] IP Header
    skb_push(transferControl.socketBuffer, sizeof(struct iphdr));
    transferControl.ipHeader = (struct iphdr *)transferControl.socketBuffer->data;
    transferControl.ipHeader->version = 4;
    transferControl.ipHeader->ihl = 5;
    transferControl.ipHeader->ttl = 64;
    transferControl.ipHeader->protocol = IPPROTO_TCP;
    transferControl.ipHeader->saddr = transferControl.source_IP;
    transferControl.ipHeader->daddr = transferControl.dest_IP;
    transferControl.ipHeader->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + PAYLOAD_LEN);
    transferControl.ipHeader->check = ip_fast_csum((unsigned char *)transferControl.ipHeader, transferControl.ipHeader->ihl);

    // [L2] Ethernet Header
    skb_push(transferControl.socketBuffer, ETH_HLEN);
    transferControl.ethernetHeader = (struct ethhdr *)transferControl.socketBuffer->data;
    memcpy(transferControl.ethernetHeader->h_dest, transferControl.destinationMAC, ETH_ALEN);
    memcpy(transferControl.ethernetHeader->h_source, networkControl.networkDevice->dev_addr, ETH_ALEN);
    transferControl.ethernetHeader->h_proto = htons(ETH_P_IP);

    // [L1] Send Packet
    transferControl.socketBuffer->dev = networkControl.networkDevice;
    transferControl.socketBuffer->protocol = transferControl.ethernetHeader->h_proto;
    transferControl.socketBuffer->pkt_type = PACKET_BROADCAST;
    transferControl.socketBuffer->ip_summed = CHECKSUM_UNNECESSARY;

    if (dev_queue_xmit(transferControl.socketBuffer) < 0) {
        pr_err("[TX][TCP][L1] Failed to transmit TCP segment\n");
        kfree_skb(transferControl.socketBuffer);
        return -EIO;
    }

    pr_info("[TX][TCP][L1] Encrypted TCP segment sent\n");
    return 0;
}

int arpSendRequest(void)
{
    struct arpHeader *arp;
    __be32 target_ip = in_aton(TARGET_IP);
    __be32 source_ip = in_aton(SRC_IP);
    int arp_len = sizeof(struct arpHeader);
    int total_len = ETH_HLEN + arp_len;

    if (!networkControl.networkDevice) {
        pr_err("[TX][ARP] Network device not initialized\n");
        return -ENODEV;
    }

    if (!(networkControl.networkDevice->flags & IFF_UP)) {
        pr_err("[TX][ARP] Interface %s is down\n", networkControl.iface_name);
        return -ENETDOWN;
    }

    if (!is_valid_ether_addr(networkControl.networkDevice->dev_addr)) {
        pr_err("[TX][ARP] Invalid source MAC address\n");
        return -EINVAL;
    }

    transferControl.socketBuffer = alloc_skb(total_len + NET_IP_ALIGN, GFP_ATOMIC);
    if (!transferControl.socketBuffer)
        return -ENOMEM;

    skb_reserve(transferControl.socketBuffer, NET_IP_ALIGN + ETH_HLEN);

    arp = (struct arpHeader *)skb_put(transferControl.socketBuffer, arp_len);
    memset(arp, 0, arp_len);

    // Fill ARP header
    arp->ar_hrd = htons(ARPHRD_ETHER);
    arp->ar_pro = htons(ETH_P_IP);
    arp->ar_hln = ETH_ALEN;
    arp->ar_pln = 4;
    arp->ar_op  = htons(ARPOP_REQUEST);

    memcpy(arp->ar_sha, networkControl.networkDevice->dev_addr, ETH_ALEN);
    arp->ar_sip = source_ip;

    memset(arp->ar_tha, 0x00, ETH_ALEN);
    arp->ar_tip = target_ip;

    // Ethernet header
    skb_push(transferControl.socketBuffer, ETH_HLEN);
    transferControl.ethernetHeader = (struct ethhdr *)transferControl.socketBuffer->data;

    transferControl.ethernetHeader->h_proto = htons(ETH_P_ARP);
    memcpy(transferControl.ethernetHeader->h_source, networkControl.networkDevice->dev_addr, ETH_ALEN);
    memset(transferControl.ethernetHeader->h_dest, 0xFF, ETH_ALEN); // Broadcast

    transferControl.socketBuffer->dev = networkControl.networkDevice;
    transferControl.socketBuffer->protocol = transferControl.ethernetHeader->h_proto;
    transferControl.socketBuffer->pkt_type = PACKET_BROADCAST;
    transferControl.socketBuffer->ip_summed = CHECKSUM_UNNECESSARY;

    if (dev_queue_xmit(transferControl.socketBuffer) < 0) {
        pr_err("[TX][ARP] Failed to transmit ARP request\n");
        kfree_skb(transferControl.socketBuffer);
        return -EIO;
    }

    pr_info("[TX][ARP] ARP request sent for IP %pI4\n", &target_ip);

    return 0;
}

static int configureNetworkDevice(void)
{
    networkControl.networkDevice = dev_get_by_name(&init_net, networkControl.iface_name);
    if (!networkControl.networkDevice)
    {
        pr_err("[ERNO][NET] Device %s not found\n", networkControl.iface_name);
        return -ENODEV;
    }

    if (!(networkControl.networkDevice->flags & IFF_UP))
    {
        pr_err("[ERNO][NET] Interface %s is down\n", networkControl.iface_name);
        dev_put(networkControl.networkDevice);
        return -ENETDOWN;
    }

    if (!is_valid_ether_addr(networkControl.networkDevice->dev_addr))
    {
        pr_err("[ERNO][NET] Invalid MAC address on %s\n", networkControl.iface_name);
        dev_put(networkControl.networkDevice);
        return -EINVAL;
    }

    return 0;
}

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

    // if (state->in || state->out)
    // {
    //     pr_info("[L2][ETH] In: %s | Out: %s\n", state->in ? state->in->name : "N/A", state->out ? state->out->name : "N/A");

    //     if (state->in && strcmp(state->in->name, "eth0") == 0)
    //     {
    //         pr_info("[L2][ETH] Packet received on interface: %s\n", state->in->name);
    //     }
    // }

    if (skb_mac_header_was_set(socketBuffer))
    {
        hookControl.ethernetHeader = eth_hdr(socketBuffer);
        // if (hookControl.ethernetHeader)
        // {
        //     pr_info("[L2][ETH] Src MAC: %pM -> Dst MAC: %pM | Proto: 0x%04x\n", hookControl.ethernetHeader->h_source, hookControl.ethernetHeader->h_dest, ntohs(hookControl.ethernetHeader->h_proto));
        // }
    }

    iph = ip_hdr(socketBuffer);
    if (!iph)
    {
        return NF_ACCEPT;
    }

    // pr_info("[L3][IP] Src IP: %pI4 -> Dst IP: %pI4 | Proto: %d\n", &iph->saddr, &iph->daddr, iph->protocol);

    if (iph->protocol == IPPROTO_UDP)
    {
        udph = udp_hdr(socketBuffer);
        if (!udph)
        {
            return NF_ACCEPT;
        }

        dport = ntohs(udph->dest);
        if (dport != LISTEN_PORT && dport != TEST_PORT)
        {
            return NF_ACCEPT;
        }

        if (iph->daddr != htonl(BROADCAST_IP))
        {
            return NF_ACCEPT;
        }

        payload = (u8 *)udph + sizeof(struct udphdr);
        payload_len = ntohs(udph->len) - sizeof(struct udphdr);

        // pr_info("[L4][UDP] Src Port: %d -> Dst Port: %d\n", ntohs(udph->source), dport);

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
        {
            return NF_ACCEPT;
        }

        payload = (u8 *)tcph + (tcph->doff * 4);
        payload_len = ntohs(iph->tot_len) - (iph->ihl * 4) - (tcph->doff * 4);

        // pr_info("[L4][TCP] Src Port: %d -> Dst Port: %d\n", ntohs(tcph->source), ntohs(tcph->dest));

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

static void registerNetFilterHook(void)
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
}

static void registerHandlerARP(void)
{
    // Register ARP RX handler
    arp_packet_type.type = htons(ETH_P_ARP);
    arp_packet_type.func = arpResponse;
    arp_packet_type.dev = networkControl.networkDevice;
    arp_packet_type.af_packet_priv = NULL;
    dev_add_pack(&arp_packet_type);
}

int networkInit(void)
{
    pr_info("[INIT][NET] Master Controller Network Initialisation\n");

    (void)configureNetworkDevice();
    registerNetFilterHook();
    registerHandlerARP();

    return 0;
}

void networkDestroy(void)
{
    nf_unregister_net_hook(&init_net, &netFilterHook);

    if (networkControl.networkDevice)
    {
        dev_remove_pack(&arp_packet_type);
        dev_put(networkControl.networkDevice);
        networkControl.networkDevice = NULL;
    }

    pr_info("[TX][DESTROY] IceNET Master Controller Module Unloaded\n");
}
