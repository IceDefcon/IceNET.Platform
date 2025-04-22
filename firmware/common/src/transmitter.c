/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "transmitter.h"
#include "crypto.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// [L7] Application Layer: Static payload encryption key
static const unsigned char aes_key[AES_KEY_LEN] =
{
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x97, 0x75, 0x46, 0x7e, 0x56, 0x4e
};

static networkControlType networkControl =
{
    .networkDevice = NULL,
    .iface_name = "Unknown",
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* GET */ networkControlType* getNetworkController(void)
{
    return &networkControl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
    transferControl.tcpHeader->source = htons(44000);
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

    if (!(networkControl.networkDevice->flags & IFF_UP))
    {
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
