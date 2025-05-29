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

static transmissionControlType transmissionControl =
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
    .target_IP = 0
};

static arpHeaderType arpHeader =
{
    .ar_hrd = 0,    // Hardware type
    .ar_pro = 0,    // Protocol type
    .ar_hln = 6,    // Hardware address length
    .ar_pln = 4,    // Protocol address length
    .ar_op = 0,     // Opcode (request/reply)
    .ar_sha = {0},  // Sender MAC
    .ar_sip = 0,    // Sender IP
    .ar_tha = {0},  // Target MAC
    .ar_tip = 0     // Target IP
};

static ndpType ndpPacket =
{
    .icmph =
    {
        .icmp6_type = 0,
        .icmp6_code = 0,
        .icmp6_cksum = 0,
        .icmp6_unused = 0
    },

    .target = IN6ADDR_ANY_INIT,  // This sets all 16 bytes to 0
    .opt =
    {
        .type = 0,
        .length = 0,
        .addr = {0, 0, 0, 0, 0, 0}
    }
};

static ndpRequestType ndpRequest =
{
    .dest_addr = { 0 },
    .sock = NULL,
    .msg = { 0 },
    .vec = { 0 },
    .target_ipv6 = IN6ADDR_ANY_INIT,
    .solicited_node_multicast = IN6ADDR_ANY_INIT,
    .dev = NULL,
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* GET */ networkControlType* getNetworkController(void)
{
    return &networkControl;
}

/* CLEAR */ static void clearTransmissionControl(void)
{
    int i;

    transmissionControl.socketBuffer = NULL;
    transmissionControl.ethernetHeader = NULL;
    transmissionControl.ipHeader = NULL;
    transmissionControl.udpHeader = NULL;
    transmissionControl.tcpHeader = NULL;
    transmissionControl.Data[DATA_PACKET_UDP] = NULL;
    transmissionControl.Data[DATA_PACKET_TCP] = NULL;
    transmissionControl.transmissionLength = 0;

    for (i = 0; i < ETH_ALEN; i++)
    {
        transmissionControl.destinationMAC[i] = 0;
    }

    transmissionControl.source_IP = 0;
    transmissionControl.target_IP = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int icmpSendPing(__be32 dest_ip)
{
    struct socket *sock = NULL;
    struct sockaddr_in addr;
    struct msghdr msg;
    struct kvec iov;
    struct icmphdr *icmp;
    char *packet;
    int ret;
    int packet_size = sizeof(struct icmphdr);

    packet = kmalloc(packet_size, GFP_KERNEL);
    if (!packet)
    {
        return -ENOMEM;
    }

    ret = sock_create_kern(&init_net, AF_INET, SOCK_RAW, IPPROTO_ICMP, &sock);
    if (ret < 0)
    {
        printk(KERN_ERR "[TX][ICMP] Failed to create socket: %d\n", ret);
        kfree(packet);
        return ret;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = dest_ip;

    icmp = (struct icmphdr *)packet;
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->un.echo.id = htons(0x1234);
    icmp->un.echo.sequence = htons(1);
    icmp->checksum = ip_compute_csum((void *)icmp, packet_size);

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &addr;
    msg.msg_namelen = sizeof(addr);

    iov.iov_base = packet;
    iov.iov_len = packet_size;

    ret = kernel_sendmsg(sock, &msg, &iov, 1, packet_size);
    if (ret < 0)
    {
        printk(KERN_ERR "[TX][ICMP] Failed to send ICMP echo request: %d\n", ret);
    }
    else
    {
        printk(KERN_INFO "[TX][ICMP] ICMP Echo Request sent to %pI4\n", &dest_ip);
    }

    sock_release(sock);
    kfree(packet);

    return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int udpTransmission(void)
{
    transmissionControl.transmissionLength = ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr) + PAYLOAD_LEN;

    // [L2] Data Link Layer: Set destination to broadcast MAC
    memset(transmissionControl.destinationMAC, 0xFF, ETH_ALEN);

    // [L3] Network Layer
    transmissionControl.source_IP = in_aton(TRANSMITTER_IP);
    transmissionControl.target_IP = in_aton(BROADCAST_IP);

    // [L2] Memory allocation for socket buffer
    transmissionControl.socketBuffer = alloc_skb(transmissionControl.transmissionLength + NET_IP_ALIGN, GFP_ATOMIC);
    if (!transmissionControl.socketBuffer)
    {
        return -ENOMEM;
    }

    skb_reserve(transmissionControl.socketBuffer, NET_IP_ALIGN);
    skb_reserve(transmissionControl.socketBuffer, ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr));

    // [L7] Application Layer: Write raw payload
    transmissionControl.Data[DATA_PACKET_UDP] = skb_put(transmissionControl.socketBuffer, PAYLOAD_LEN);
    memcpy(transmissionControl.Data[DATA_PACKET_UDP], "Ice.Marek.IceNET.Technology.x-86", PAYLOAD_LEN);

    if (aesEncrypt(transmissionControl.Data[DATA_PACKET_UDP], PAYLOAD_LEN, (u8 *)aes_key, NULL) < 0)
    {
        pr_err("[TX][UDP][L6] AES encryption failed\n");
        kfree_skb(transmissionControl.socketBuffer);
        return -EIO;
    }

    print_hex_dump(KERN_INFO, "[TX][UDP][L7] Encrypted Payload: ", DUMP_PREFIX_OFFSET, 16, 1, transmissionControl.Data[DATA_PACKET_UDP], PAYLOAD_LEN, false);

    // [L4] Transport Layer: UDP header setup
    skb_push(transmissionControl.socketBuffer, sizeof(struct udphdr));
    transmissionControl.udpHeader = (struct udphdr *)transmissionControl.socketBuffer->data;
    transmissionControl.udpHeader->source = htons(TRANSMITTER_PORT);
    transmissionControl.udpHeader->dest = htons(RECEIVER_PORT);
    transmissionControl.udpHeader->len = htons(sizeof(struct udphdr) + PAYLOAD_LEN);
    transmissionControl.udpHeader->check = 0;

    // [L3] Network Layer: IP header setup
    skb_push(transmissionControl.socketBuffer, sizeof(struct iphdr));
    transmissionControl.ipHeader = (struct iphdr *)transmissionControl.socketBuffer->data;
    transmissionControl.ipHeader->version = 4;
    transmissionControl.ipHeader->ihl = 5;
    transmissionControl.ipHeader->tos = 0;
    transmissionControl.ipHeader->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + PAYLOAD_LEN);
    transmissionControl.ipHeader->id = 0;
    transmissionControl.ipHeader->frag_off = 0;
    transmissionControl.ipHeader->ttl = 64;
    transmissionControl.ipHeader->protocol = IPPROTO_UDP;
    transmissionControl.ipHeader->saddr = transmissionControl.source_IP;
    transmissionControl.ipHeader->daddr = transmissionControl.target_IP;
    transmissionControl.ipHeader->check = ip_fast_csum((unsigned char *)transmissionControl.ipHeader, transmissionControl.ipHeader->ihl);

    // [L2] Ethernet header setup
    skb_push(transmissionControl.socketBuffer, ETH_HLEN);
    transmissionControl.ethernetHeader = (struct ethhdr *)transmissionControl.socketBuffer->data;
    memcpy(transmissionControl.ethernetHeader->h_dest, transmissionControl.destinationMAC, ETH_ALEN);
    memcpy(transmissionControl.ethernetHeader->h_source, networkControl.networkDevice->dev_addr, ETH_ALEN);
    transmissionControl.ethernetHeader->h_proto = htons(ETH_P_IP);

    // [L1] Physical Layer (abstracted in kernel): Queue packet for transmission
    transmissionControl.socketBuffer->dev = networkControl.networkDevice;
    transmissionControl.socketBuffer->protocol = transmissionControl.ethernetHeader->h_proto;
    transmissionControl.socketBuffer->pkt_type = PACKET_BROADCAST;
    transmissionControl.socketBuffer->ip_summed = CHECKSUM_UNNECESSARY;

    if (dev_queue_xmit(transmissionControl.socketBuffer) < 0)
    {
        pr_err("[TX][UDP][L1] Failed to transmit UDP packet\n");
        kfree_skb(transmissionControl.socketBuffer);
        return -EIO;
    }

    pr_info("[TX][UDP][L1] Encrypted UDP packet sent to broadcast\n");
    clearTransmissionControl();

    return 0;
}

int tcpTransmission(void)
{
    transmissionControl.transmissionLength = ETH_HLEN + sizeof(struct iphdr) + sizeof(struct tcphdr) + PAYLOAD_LEN;

    // [L2] Data Link Layer: Set destination to broadcast MAC
    memset(transmissionControl.destinationMAC, 0xFF, ETH_ALEN);

    // [L3] Network Layer
    transmissionControl.source_IP = in_aton(TRANSMITTER_IP);
    transmissionControl.target_IP = in_aton(BROADCAST_IP);

    transmissionControl.socketBuffer = alloc_skb(transmissionControl.transmissionLength + NET_IP_ALIGN, GFP_ATOMIC);
    if (!transmissionControl.socketBuffer) return -ENOMEM;

    skb_reserve(transmissionControl.socketBuffer, NET_IP_ALIGN);
    skb_reserve(transmissionControl.socketBuffer, ETH_HLEN + sizeof(struct iphdr) + sizeof(struct tcphdr));

    // [L7] Application Layer: Payload
    transmissionControl.Data[DATA_PACKET_TCP] = skb_put(transmissionControl.socketBuffer, PAYLOAD_LEN);
    memcpy(transmissionControl.Data[DATA_PACKET_TCP], "Ice.Marek.IceNET.Technology.x-86", PAYLOAD_LEN);

    if (aesEncrypt(transmissionControl.Data[DATA_PACKET_TCP], PAYLOAD_LEN, (u8 *)aes_key, NULL) < 0)
    {
        pr_err("[TX][TCP][L6] AES encryption failed\n");
        kfree_skb(transmissionControl.socketBuffer);
        return -EIO;
    }

    print_hex_dump(KERN_INFO, "[TX][TCP][L7] Encrypted Payload: ", DUMP_PREFIX_OFFSET, 16, 1, transmissionControl.Data[DATA_PACKET_TCP], PAYLOAD_LEN, false);

    // [L4] TCP Header
    skb_push(transmissionControl.socketBuffer, sizeof(struct tcphdr));
    transmissionControl.tcpHeader = (struct tcphdr *)transmissionControl.socketBuffer->data;
    memset(transmissionControl.tcpHeader, 0, sizeof(struct tcphdr));
    transmissionControl.tcpHeader->source = htons(44000);
    transmissionControl.tcpHeader->dest = htons(80);
    transmissionControl.tcpHeader->seq = htonl(0);
    transmissionControl.tcpHeader->doff = 5;
    transmissionControl.tcpHeader->syn = 1;
    transmissionControl.tcpHeader->window = htons(1024);
    transmissionControl.tcpHeader->check = 0;

    // [L3] IP Header
    skb_push(transmissionControl.socketBuffer, sizeof(struct iphdr));
    transmissionControl.ipHeader = (struct iphdr *)transmissionControl.socketBuffer->data;
    transmissionControl.ipHeader->version = 4;
    transmissionControl.ipHeader->ihl = 5;
    transmissionControl.ipHeader->ttl = 64;
    transmissionControl.ipHeader->protocol = IPPROTO_TCP;
    transmissionControl.ipHeader->saddr = transmissionControl.source_IP;
    transmissionControl.ipHeader->daddr = transmissionControl.target_IP;
    transmissionControl.ipHeader->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + PAYLOAD_LEN);
    transmissionControl.ipHeader->check = ip_fast_csum((unsigned char *)transmissionControl.ipHeader, transmissionControl.ipHeader->ihl);

    // [L2] Ethernet Header
    skb_push(transmissionControl.socketBuffer, ETH_HLEN);
    transmissionControl.ethernetHeader = (struct ethhdr *)transmissionControl.socketBuffer->data;
    memcpy(transmissionControl.ethernetHeader->h_dest, transmissionControl.destinationMAC, ETH_ALEN);
    memcpy(transmissionControl.ethernetHeader->h_source, networkControl.networkDevice->dev_addr, ETH_ALEN);
    transmissionControl.ethernetHeader->h_proto = htons(ETH_P_IP);

    // [L1] Send Packet
    transmissionControl.socketBuffer->dev = networkControl.networkDevice;
    transmissionControl.socketBuffer->protocol = transmissionControl.ethernetHeader->h_proto;
    transmissionControl.socketBuffer->pkt_type = PACKET_BROADCAST;
    transmissionControl.socketBuffer->ip_summed = CHECKSUM_UNNECESSARY;

    if (dev_queue_xmit(transmissionControl.socketBuffer) < 0) {
        pr_err("[TX][TCP][L1] Failed to transmit TCP segment\n");
        kfree_skb(transmissionControl.socketBuffer);
        return -EIO;
    }

    pr_info("[TX][TCP][L1] Encrypted TCP segment sent\n");
    clearTransmissionControl();

    return 0;
}

int arpSendRequest(void)
{
    transmissionControl.source_IP = in_aton(TRANSMITTER_IP);
    transmissionControl.target_IP = in_aton(RECEIVER_IP);
    transmissionControl.transmissionLength = ETH_HLEN + sizeof(arpHeaderType);;

    if (!networkControl.networkDevice)
    {
        pr_err("[TX][ARP] Network device not initialized\n");
        return -ENODEV;
    }

    if (!(networkControl.networkDevice->flags & IFF_UP))
    {
        pr_err("[TX][ARP] Interface %s is down\n", networkControl.iface_name);
        return -ENETDOWN;
    }

    if (!is_valid_ether_addr(networkControl.networkDevice->dev_addr))
    {
        pr_err("[TX][ARP] Invalid source MAC address\n");
        return -EINVAL;
    }

    transmissionControl.socketBuffer = alloc_skb(transmissionControl.transmissionLength + NET_IP_ALIGN, GFP_ATOMIC);
    if (!transmissionControl.socketBuffer)
    {
        return -ENOMEM;
    }

    skb_reserve(transmissionControl.socketBuffer, NET_IP_ALIGN + ETH_HLEN);

    // Prepare ARP header statically
    arpHeader.ar_hrd = htons(ARPHRD_ETHER);
    arpHeader.ar_pro = htons(ETH_P_IP);
    arpHeader.ar_hln = ETH_ALEN;
    arpHeader.ar_pln = 4;
    arpHeader.ar_op  = htons(ARPOP_REQUEST);

    memcpy(arpHeader.ar_sha, networkControl.networkDevice->dev_addr, ETH_ALEN);
    arpHeader.ar_sip = transmissionControl.source_IP;

    memset(arpHeader.ar_tha, 0x00, ETH_ALEN);
    arpHeader.ar_tip = transmissionControl.target_IP;

    // Copy it to the skb
    memcpy(skb_put(transmissionControl.socketBuffer, sizeof(arpHeader)), &arpHeader, sizeof(arpHeader));

    // Ethernet header
    skb_push(transmissionControl.socketBuffer, ETH_HLEN);
    transmissionControl.ethernetHeader = (struct ethhdr *)transmissionControl.socketBuffer->data;

    transmissionControl.ethernetHeader->h_proto = htons(ETH_P_ARP);
    memcpy(transmissionControl.ethernetHeader->h_source, networkControl.networkDevice->dev_addr, ETH_ALEN);
    memset(transmissionControl.ethernetHeader->h_dest, 0xFF, ETH_ALEN); // Broadcast

    transmissionControl.socketBuffer->dev = networkControl.networkDevice;
    transmissionControl.socketBuffer->protocol = transmissionControl.ethernetHeader->h_proto;
    transmissionControl.socketBuffer->pkt_type = PACKET_BROADCAST;
    transmissionControl.socketBuffer->ip_summed = CHECKSUM_UNNECESSARY;

    if (dev_queue_xmit(transmissionControl.socketBuffer) < 0) {
        pr_err("[TX][ARP] Failed to transmit ARP request\n");
        kfree_skb(transmissionControl.socketBuffer);
        return -EIO;
    }

    pr_info("[TX][ARP] ARP request sent for IP %pI4\n", &transmissionControl.target_IP);
    clearTransmissionControl();

    return 0;
}

int ndpSendRequest(void)
{
    int ret;

    pr_info("[TX][NDP] Starting NDP request\n");

    // Create raw socket for ICMPv6
    ret = sock_create(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6, &ndpRequest.sock);
    if (ret < 0)
    {
        pr_err("[TX][NDP] Failed to create socket\n");
        return ret;
    }

    // Parse the target IPv6 address you're trying to resolve
    in6_pton("fe80::d1d2:2209:4803:5ca2", -1, (u8 *)&ndpRequest.target_ipv6, 0, NULL);

    // Construct the solicited-node multicast address: ff02::1:ffXX:XXXX
    memset(&ndpRequest.solicited_node_multicast, 0, sizeof(ndpRequest.solicited_node_multicast));
    ndpRequest.solicited_node_multicast.s6_addr[0]  = 0xff;
    ndpRequest.solicited_node_multicast.s6_addr[1]  = 0x02;
    ndpRequest.solicited_node_multicast.s6_addr[11] = 0x01;
    ndpRequest.solicited_node_multicast.s6_addr[12] = 0xff;
    ndpRequest.solicited_node_multicast.s6_addr[13] = ndpRequest.target_ipv6.s6_addr[13];
    ndpRequest.solicited_node_multicast.s6_addr[14] = ndpRequest.target_ipv6.s6_addr[14];
    ndpRequest.solicited_node_multicast.s6_addr[15] = ndpRequest.target_ipv6.s6_addr[15];

    // Get device and its index
    ndpRequest.dev = dev_get_by_name(&init_net, "wlp2s0");
    if (!ndpRequest.dev)
    {
        pr_err("[TX][NDP] Failed to get net_device for wlp2s0\n");
        sock_release(ndpRequest.sock);
        return -ENODEV;
    }

    // Set destination address to the solicited-node multicast
    memset(&ndpRequest.dest_addr, 0, sizeof(ndpRequest.dest_addr));
    ndpRequest.dest_addr.sin6_family = AF_INET6;
    ndpRequest.dest_addr.sin6_addr = ndpRequest.solicited_node_multicast;
    ndpRequest.dest_addr.sin6_scope_id = ndpRequest.dev->ifindex;  // Important for link-local addresses

    // Fill in ICMPv6 Neighbor Solicitation message
    ndpPacket.icmph.icmp6_type = NDISC_NEIGHBOUR_SOLICITATION;
    ndpPacket.icmph.icmp6_code = 0;
    ndpPacket.icmph.icmp6_cksum = 0; // Kernel will calculate checksum

    ndpPacket.target = ndpRequest.target_ipv6;

    // Add Source Link-Layer Address Option (Type 1)
    ndpPacket.opt.type = 1;
    ndpPacket.opt.length = 1;
    memcpy(ndpPacket.opt.addr, ndpRequest.dev->dev_addr, ETH_ALEN); // Use actual MAC of wlp2s0

    dev_put(ndpRequest.dev); // Release reference to net_device

    // Prepare vector and message
    ndpRequest.vec.iov_base = &ndpPacket;
    ndpRequest.vec.iov_len = sizeof(ndpPacket);

    memset(&ndpRequest.msg, 0, sizeof(ndpRequest.msg));
    ndpRequest.msg.msg_name = &ndpRequest.dest_addr;
    ndpRequest.msg.msg_namelen = sizeof(ndpRequest.dest_addr);

    // Send the message
    ret = kernel_sendmsg(ndpRequest.sock, &ndpRequest.msg, &ndpRequest.vec, 1, ndpRequest.vec.iov_len);
    if (ret < 0)
    {
        pr_err("[TX][NDP] Failed to send NDP request\n");
    }
    else
    {
        pr_info("[TX][NDP] NDP request sent to %pI6\n", &ndpRequest.dest_addr.sin6_addr);
    }

    sock_release(ndpRequest.sock);
    clearTransmissionControl();

    return ret;
}
