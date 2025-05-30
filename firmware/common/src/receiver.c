/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "diagnostics.h"
#include "receiver.h"
#include "crypto.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static hookDiagnosticsType hookDiagnostics =
{
    .icmpHeader = NULL,
    .icmpHeaderReply = NULL,
    .socketBufferReply = NULL,
    .ipHeaderReply = NULL,
    .ethernetHeader = NULL,
    .ethernetHeaderReply = NULL,
    .dataBuffer = NULL,
    .dataBufferLength = 0,
    .ipHeader = NULL,
};

static hookCommunicationType hookCommunication =
{
    .ipHeader = NULL,
    .udpHeader = NULL,
    .tcpHeader = NULL,
    .payload = NULL,
    .payloadLength = 0,
    .destPort = 0,
    .aesKey =
    {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x97, 0x75, 0x46, 0x7e, 0x56, 0x4e
    },
};

static arpRequestType arpRequest =
{
    .arpHeader = NULL,
    .arpPtr = NULL,
    .socketBufferReply = NULL,
    .networkDevice = NULL,
    .senderMacAddress = NULL,
    .senderIpAddress = NULL,
    .targetMacAddress = NULL,
    .targetIpAddress = NULL,
    .arpReplyPtr = NULL,
    .ethernetHeader = NULL,
    .arpHeaderLength = 0,
    .senderIp = 0,
    .targetIp = 0,
    .ourMac = {0},
    .ourIp = 0,
    .allowedSenderIp = 0,
};

static int RX_Count = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int receiverHookDiagnostic(void *priv, struct sk_buff *socketBuffer, const struct nf_hook_state *state)
{
    if (!socketBuffer)
    {
        return NF_ACCEPT;
    }

    hookDiagnostics.ipHeader = ip_hdr(socketBuffer);
    if (!hookDiagnostics.ipHeader)
    {
        return NF_ACCEPT;
    }

    if (hookDiagnostics.ipHeader->protocol == IPPROTO_ICMP)
    {
        hookDiagnostics.icmpHeader = icmp_hdr(socketBuffer);
        if (!hookDiagnostics.icmpHeader)
        {
            return NF_ACCEPT;
        }
/**
 * This can be commented
 * as Linux operating system
 * handles response to ICMP packets
 */
#if 0
        if (hookDiagnostics.icmpHeader->type == ICMP_ECHO)
        {
            hookDiagnostics.dataBufferLength = ntohs(hookDiagnostics.ipHeader->tot_len) - (hookDiagnostics.ipHeader->ihl * 4) - sizeof(struct icmphdr);
            if (hookDiagnostics.dataBufferLength < 0)
            {
                return NF_ACCEPT;
            }

            hookDiagnostics.socketBufferReply = alloc_skb(ETH_HLEN + ntohs(hookDiagnostics.ipHeader->tot_len), GFP_ATOMIC);
            if (!hookDiagnostics.socketBufferReply)
            {
                return NF_ACCEPT;
            }

            skb_reserve(hookDiagnostics.socketBufferReply, ETH_HLEN);
            hookDiagnostics.dataBuffer = skb_put(hookDiagnostics.socketBufferReply, ntohs(hookDiagnostics.ipHeader->tot_len));

            hookDiagnostics.ipHeaderReply = (struct iphdr *)hookDiagnostics.dataBuffer;
            memcpy(hookDiagnostics.ipHeaderReply, hookDiagnostics.ipHeader, hookDiagnostics.ipHeader->ihl * 4);
            hookDiagnostics.ipHeaderReply->saddr = hookDiagnostics.ipHeader->daddr;
            hookDiagnostics.ipHeaderReply->daddr = hookDiagnostics.ipHeader->saddr;
            hookDiagnostics.ipHeaderReply->ttl = 64;
            hookDiagnostics.ipHeaderReply->check = 0;
            hookDiagnostics.ipHeaderReply->check = ip_fast_csum((unsigned char *)hookDiagnostics.ipHeaderReply, hookDiagnostics.ipHeaderReply->ihl);

            hookDiagnostics.icmpHeaderReply = (struct icmphdr *)(hookDiagnostics.dataBuffer + (hookDiagnostics.ipHeaderReply->ihl * 4));
            memcpy(hookDiagnostics.icmpHeaderReply, hookDiagnostics.icmpHeader, sizeof(struct icmphdr) + hookDiagnostics.dataBufferLength);
            hookDiagnostics.icmpHeaderReply->type = ICMP_ECHOREPLY;
            hookDiagnostics.icmpHeaderReply->checksum = 0;
            hookDiagnostics.icmpHeaderReply->checksum = ip_compute_csum((void *)hookDiagnostics.icmpHeaderReply, sizeof(struct icmphdr) + hookDiagnostics.dataBufferLength);

            hookDiagnostics.socketBufferReply->dev = socketBuffer->dev;
            hookDiagnostics.socketBufferReply->protocol = htons(ETH_P_IP);
            skb_reset_mac_header(hookDiagnostics.socketBufferReply);
            skb_reset_network_header(hookDiagnostics.socketBufferReply);

            hookDiagnostics.ethernetHeader = eth_hdr(socketBuffer);
            hookDiagnostics.ethernetHeaderReply = (struct ethhdr *)skb_push(hookDiagnostics.socketBufferReply, ETH_HLEN);
            memcpy(hookDiagnostics.ethernetHeaderReply->h_dest, hookDiagnostics.ethernetHeader->h_source, ETH_ALEN);
            memcpy(hookDiagnostics.ethernetHeaderReply->h_source, hookDiagnostics.ethernetHeader->h_dest, ETH_ALEN);
            hookDiagnostics.ethernetHeaderReply->h_proto = htons(ETH_P_IP);

            dev_queue_xmit(hookDiagnostics.socketBufferReply);

            pr_info("[RX][ICMP] Echo Reply sent to %pI4 (id=%u, seq=%u)\n", &hookDiagnostics.ipHeader->saddr, ntohs(hookDiagnostics.icmpHeader->un.echo.id), ntohs(hookDiagnostics.icmpHeader->un.echo.sequence));

            return NF_STOLEN;
        }
        else
#else
        if (hookDiagnostics.icmpHeader->type == ICMP_ECHOREPLY)
        {
            __be32 src_ip = hookDiagnostics.ipHeader->saddr;
            __be16 id = hookDiagnostics.icmpHeader->un.echo.id;
            __be16 seq = hookDiagnostics.icmpHeader->un.echo.sequence;

            pr_info("[RX][ICMP] Echo Reply received from %pI4 (id=%u, seq=%u)\n", &src_ip, ntohs(id), ntohs(seq));

            addActiveHost(src_ip);

            return NF_ACCEPT;
        }
#endif
    }

    return NF_ACCEPT;
}

unsigned int receiverHookCommunication(void *priv, struct sk_buff *socketBuffer, const struct nf_hook_state *state)
{
    if (!socketBuffer)
    {
        return NF_ACCEPT;
    }

    hookCommunication.ipHeader = ip_hdr(socketBuffer);
    if (!hookCommunication.ipHeader)
    {
        return NF_ACCEPT;
    }

    if (hookCommunication.ipHeader->protocol == IPPROTO_UDP)
    {
        hookCommunication.udpHeader = udp_hdr(socketBuffer);
        if (!hookCommunication.udpHeader)
        {
            return NF_ACCEPT;
        }

        hookCommunication.destPort = ntohs(hookCommunication.udpHeader->dest);
        if (hookCommunication.destPort != UDP_PORT)
        {
            return NF_ACCEPT;
        }

        if (hookCommunication.ipHeader->daddr != htonl(BROADCAST_HEX_IP))
        {
            return NF_ACCEPT;
        }

        hookCommunication.payload = (u8 *)hookCommunication.udpHeader + sizeof(struct udphdr);
        hookCommunication.payloadLength = ntohs(hookCommunication.udpHeader->len) - sizeof(struct udphdr);

        // pr_info("[L4][UDP] Src Port: %d -> Dst Port: %d\n", ntohs(hookCommunication.udpHeader->source), hookCommunication.destPort);

        if (hookCommunication.payloadLength > 0 && skb_tail_pointer(socketBuffer) >= hookCommunication.payload + hookCommunication.payloadLength)
        {
            // pr_info("[L7][DATA][UDP] Encrypted Payload: %.32s\n", hookCommunication.payload);
            if (aesDecrypt(hookCommunication.payload, hookCommunication.payloadLength, hookCommunication.aesKey, NULL) == 0)
            {
                if('I' == hookCommunication.payload[0] && 'c' == hookCommunication.payload[1] && 'e' == hookCommunication.payload[2])
                {
                    pr_info("[L4][UDP] Src Port: %d -> Dst Port: %d\n", ntohs(hookCommunication.udpHeader->source), hookCommunication.destPort);
                    pr_info("[L7][DATA][UDP] Decrypted Payload: %.32s\n", hookCommunication.payload);
                }
            }
            else
            {
                // pr_err("[L7][DATA][UDP] Decryption failed\n");
            }
        }
        else
        {
            // pr_info("[L7][DATA][UDP] Payload not safely accessible\n");
        }
    }
    else if (hookCommunication.ipHeader->protocol == IPPROTO_TCP)
    {
        hookCommunication.tcpHeader = (struct tcphdr *)((__u32 *)hookCommunication.ipHeader + hookCommunication.ipHeader->ihl);
        if (!hookCommunication.tcpHeader)
        {
            return NF_ACCEPT;
        }

        hookCommunication.destPort = ntohs(hookCommunication.tcpHeader->dest);
        if (hookCommunication.destPort != TCP_PORT)
        {
            return NF_ACCEPT;
        }

        hookCommunication.payload = (u8 *)hookCommunication.tcpHeader + (hookCommunication.tcpHeader->doff * 4);
        hookCommunication.payloadLength = ntohs(hookCommunication.ipHeader->tot_len) - (hookCommunication.ipHeader->ihl * 4) - (hookCommunication.tcpHeader->doff * 4);

        // pr_info("[L4][TCP] Src Port: %d -> Dst Port: %d\n", ntohs(hookCommunication.tcpHeader->source), ntohs(hookCommunication.tcpHeader->dest));

        if (hookCommunication.payloadLength > 0 && skb_tail_pointer(socketBuffer) >= hookCommunication.payload + hookCommunication.payloadLength)
        {
            // pr_info("[L7][DATA][TCP] Encrypted Payload: %.32s\n", hookCommunication.payload);
            if (aesDecrypt(hookCommunication.payload, hookCommunication.payloadLength, hookCommunication.aesKey, NULL) == 0)
            {
                if('I' == hookCommunication.payload[0] && 'c' == hookCommunication.payload[1] && 'e' == hookCommunication.payload[2])
                {
                    pr_info("[L4][TCP] Src Port: %d -> Dst Port: %d\n", ntohs(hookCommunication.tcpHeader->source), ntohs(hookCommunication.tcpHeader->dest));
                    pr_info("[L7][DATA][TCP] Decrypted Payload: %.32s\n", hookCommunication.payload);
                }
            }
            else
            {
                // pr_err("[L7][DATA][TCP] Decryption failed\n");
            }
        }
        else
        {
            // pr_info("[L7][DATA][TCP] Payload not safely accessible\n");
        }
    }

    return NF_ACCEPT;
}

static int handleArpRequest(struct sk_buff *socketBuffer)
{
    arpRequest.ourIp = htonl(0xC0A808AE);     // 192.168.8.174
    arpRequest.allowedSenderIp = htonl(0xC0A80865);  // 192.168.8.101
    arpRequest.networkDevice = socketBuffer->dev;

    // pr_info("[RX][ARP] Received ARP request, socketBuffer=%p, networkDevice=%s\n", socketBuffer, arpRequest.networkDevice->name);

    arpRequest.arpHeader = arp_hdr(socketBuffer);
    if (!arpRequest.arpHeader)
    {
        pr_err("[RX][ARP] Failed to extract ARP header from socketBuffer: %p\n", socketBuffer);
        return NET_RX_DROP;
    }

    // pr_info("[RX][ARP] Extracted ARP header: arpHeader=%p, operation=%u\n", arpRequest.arpHeader, ntohs(arpRequest.arpHeader->ar_op));

    arpRequest.arpPtr = (unsigned char *)(arpRequest.arpHeader + 1);
    arpRequest.senderMacAddress = arpRequest.arpPtr;
    arpRequest.senderIpAddress = arpRequest.senderMacAddress + arpRequest.arpHeader->ar_hln;
    arpRequest.targetMacAddress = arpRequest.senderIpAddress + arpRequest.arpHeader->ar_pln;
    arpRequest.targetIpAddress = arpRequest.targetMacAddress + arpRequest.arpHeader->ar_hln;

    if (ntohs(arpRequest.arpHeader->ar_op) != ARPOP_REQUEST)
    {
        if (ntohs(arpRequest.arpHeader->ar_op) == ARPOP_REPLY)
        {
            pr_info("[RX][ARP] Sender MAC: %pM -> %pI4\n", arpRequest.senderMacAddress, arpRequest.senderIpAddress);
            pr_info("[RX][ARP] Target MAC: %pM -> %pI4\n", arpRequest.targetMacAddress, arpRequest.targetIpAddress);
        }
        return NET_RX_SUCCESS;
    }

    memcpy(&arpRequest.senderIp, arpRequest.senderIpAddress, 4);   // Sender IP
    memcpy(&arpRequest.targetIp, arpRequest.targetIpAddress, 4);   // Target IP
    if (arpRequest.targetIp != arpRequest.ourIp || arpRequest.senderIp != arpRequest.allowedSenderIp)
    {
        // pr_info("[RX][ARP] Ignoring request from %pI4 for %pI4 (Expected IPs: sender=%pI4, target=%pI4)\n", &arpRequest.senderIp, &arpRequest.targetIp, &arpRequest.allowedSenderIp, &arpRequest.ourIp);
        return NET_RX_SUCCESS;
    }

    // pr_info("[RX][ARP] Responding to ARP request from %pI4 asking for %pI4\n", &arpRequest.senderIp, &arpRequest.targetIp);

    memcpy(arpRequest.ourMac, arpRequest.networkDevice->dev_addr, ETH_ALEN); // Our MAC
    // pr_info("[RX][ARP] Our MAC address: %pM\n", arpRequest.ourMac);

    arpRequest.arpHeaderLength = sizeof(struct arphdr) + 2 * (ETH_ALEN + 4); // hln=6, pln=4
    // pr_info("[RX][ARP] ARP header length calculated: %d\n", arpRequest.arpHeaderLength);

    arpRequest.socketBufferReply = alloc_skb(ETH_HLEN + arpRequest.arpHeaderLength, GFP_ATOMIC);
    if (!arpRequest.socketBufferReply)
    {
        pr_err("[RX][ARP] Failed to allocate socketBufferReply for ARP reply\n");
        return NET_RX_DROP;
    }
    // pr_info("[RX][ARP] Successfully allocated socketBufferReply for ARP reply: socketBufferReply=%p\n", arpRequest.socketBufferReply);

    skb_reserve(arpRequest.socketBufferReply, ETH_HLEN);               // Reserve space for Ethernet header
    skb_put(arpRequest.socketBufferReply, arpRequest.arpHeaderLength);                // Make room for ARP header
    arpRequest.ethernetHeader = (struct ethhdr *)skb_push(arpRequest.socketBufferReply, ETH_HLEN);            // Push Ethernet header

    // pr_info("[RX][ARP] socketBufferReply headroom: %d, tailroom: %d\n", skb_headroom(arpRequest.socketBufferReply), skb_tailroom(arpRequest.socketBufferReply));

    arpRequest.socketBufferReply->dev = arpRequest.networkDevice;
    arpRequest.socketBufferReply->protocol = htons(ETH_P_ARP);
    skb_reset_mac_header(arpRequest.socketBufferReply);
    skb_reset_network_header(arpRequest.socketBufferReply);

    // pr_info("[RX][ARP] MAC header set at: %p\n", skb_mac_header(arpRequest.socketBufferReply));

    arpRequest.ethernetHeader = (struct ethhdr *)skb_mac_header(arpRequest.socketBufferReply);
    memcpy(arpRequest.ethernetHeader->h_dest, arpRequest.senderMacAddress, ETH_ALEN);         // Dest: sender MAC
    memcpy(arpRequest.ethernetHeader->h_source, arpRequest.ourMac, ETH_ALEN);   // Src: our MAC
    arpRequest.ethernetHeader->h_proto = htons(ETH_P_ARP);            // Protocol type

    // pr_info("[RX][ARP] Ethernet header: dst=%pM, src=%pM, proto=0x%04x\n", arpRequest.ethernetHeader->h_dest, arpRequest.ethernetHeader->h_source, ntohs(arpRequest.ethernetHeader->h_proto));

    // Alignment check (paranoid safety for non-x86 archs)
    BUILD_BUG_ON(__alignof__(struct arphdr) > __alignof__(unsigned long));

    // Setup ARP response
    arpRequest.arpHeader = (struct arphdr *)(arpRequest.ethernetHeader + 1);
    memset(arpRequest.arpHeader, 0, sizeof(struct arphdr)); // Zero for safety

    arpRequest.arpHeader->ar_hrd = htons(ARPHRD_ETHER);
    arpRequest.arpHeader->ar_pro = htons(ETH_P_IP);
    arpRequest.arpHeader->ar_hln = ETH_ALEN;
    arpRequest.arpHeader->ar_pln = 4;
    arpRequest.arpHeader->ar_op  = htons(ARPOP_REPLY);

    // pr_info("[RX][ARP] ARP header: hrd=%u, pro=%u, hln=%d, pln=%d, op=%u\n",
    //     ntohs(arpRequest.arpHeader->ar_hrd), ntohs(arpRequest.arpHeader->ar_pro),
    //     arpRequest.arpHeader->ar_hln, arpRequest.arpHeader->ar_pln, ntohs(arpRequest.arpHeader->ar_op));

    // Fill ARP payload
    arpRequest.arpReplyPtr = (unsigned char *)(arpRequest.arpHeader + 1);
    memcpy(arpRequest.arpReplyPtr, arpRequest.ourMac, ETH_ALEN);                // Sender MAC
    memcpy(arpRequest.arpReplyPtr + ETH_ALEN, &arpRequest.ourIp, 4);            // Sender IP
    memcpy(arpRequest.arpReplyPtr + ETH_ALEN + 4, arpRequest.senderMacAddress, ETH_ALEN);     // Target MAC
    memcpy(arpRequest.arpReplyPtr + 2 * ETH_ALEN + 4, arpRequest.senderIpAddress, 4);        // Target IP

    dev_queue_xmit(arpRequest.socketBufferReply);

    pr_info("[RX][ARP] Reply Successfully Sent -> %pI4\n", &arpRequest.senderIp);
    pr_info("[RX][ARP] Reply Payload -> senderMac[%pM] senderIp[%pI4] targetMac[%pM] targetIp[%pI4]\n",
        arpRequest.ourMac, &arpRequest.ourIp, arpRequest.senderMacAddress, &arpRequest.senderIp);

    return NET_RX_SUCCESS;
}

int arpReceive(struct sk_buff *socketBuffer, struct net_device *networkDevice, struct packet_type *packetType, struct net_device *originalDevice)
{
    if (!socketBuffer)
    {
        return NET_RX_DROP;
    }

    socketBuffer = skb_share_check(socketBuffer, GFP_ATOMIC);
    if (!socketBuffer)
    {
        return NET_RX_DROP;
    }

    handleArpRequest(socketBuffer);
    kfree_skb(socketBuffer);

    return NET_RX_SUCCESS;
}

static void inet6_addr_to_str(const struct in6_addr *addr, char *str, size_t size)
{
    snprintf(str, size, "%x:%x:%x:%x:%x:%x:%x:%x",
             ntohs(addr->s6_addr16[0]), ntohs(addr->s6_addr16[1]),
             ntohs(addr->s6_addr16[2]), ntohs(addr->s6_addr16[3]),
             ntohs(addr->s6_addr16[4]), ntohs(addr->s6_addr16[5]),
             ntohs(addr->s6_addr16[6]), ntohs(addr->s6_addr16[7]));
}

int ndpReceive(struct sk_buff *socketBuffer, struct net_device *networkDevice, struct packet_type *pt, struct net_device *orig_dev)
{
    struct ipv6hdr *ip6h;
    struct icmp6hdr *icmp6;
    struct nd_msg *ndm;
    struct in6_addr target_addr;
    struct sk_buff *na_skb;
    struct ipv6hdr *na_ip6h;
    struct nd_msg *na;
    int na_msg_len;
    struct in6_addr src_addr;

    if (!socketBuffer)
    {
        // printk(KERN_INFO "[NDP HOOK] Debug 0\n");
        return NET_RX_DROP;
    }

    if (!pskb_may_pull(socketBuffer, sizeof(struct ipv6hdr)))
    {
        // printk(KERN_INFO "[NDP HOOK] Debug 1\n");
        return NET_RX_DROP;
    }

    ip6h = ipv6_hdr(socketBuffer);

    if (ip6h->nexthdr != IPPROTO_ICMPV6)
    {
        // printk(KERN_INFO "[NDP HOOK] Debug 2\n");
        return NET_RX_SUCCESS; // Not ICMPv6
    }

    if (!pskb_may_pull(socketBuffer, sizeof(struct ipv6hdr) + sizeof(struct icmp6hdr)))
    {
        // printk(KERN_INFO "[NDP HOOK] Debug 3\n");
        return NET_RX_DROP;
    }

    icmp6 = (struct icmp6hdr *)(skb_network_header(socketBuffer) + sizeof(struct ipv6hdr));

    if (icmp6->icmp6_type == ICMPV6_NEIGHBOR_SOLICITATION)
    {
        printk(KERN_INFO "[RX][NDP][%d] Neighbor Solicitation received on %s\n", RX_Count, networkDevice->name);

        ndm = (struct nd_msg *)(icmp6);
        target_addr = ndm->target;

        // Optional: Check if this host owns target_addr before responding
        if (!ipv6_chk_addr(dev_net(networkDevice), &target_addr, networkDevice, 0))
        {
            // printk(KERN_INFO "[NDP HOOK] Not our address, ignoring NS\n");
            // printk(KERN_INFO "[NDP HOOK] Target address is %pI6c\n", &target_addr);
            return NET_RX_SUCCESS;
        }
        else
        {
            printk(KERN_INFO "[RX][NDP][%d] We own the target address: %pI6c on %s\n",RX_Count, &target_addr, networkDevice->name);
        }

        na_msg_len = sizeof(struct nd_msg);

        na_skb = alloc_skb(LL_RESERVED_SPACE(networkDevice) + sizeof(struct ipv6hdr) + na_msg_len, GFP_ATOMIC);
        if (!na_skb)
        {
            printk(KERN_ERR "[RX][NDP][%d] Failed to allocate skb for NA\n",RX_Count);
            return NET_RX_DROP;
        }

        skb_reserve(na_skb, LL_RESERVED_SPACE(networkDevice));
        skb_reset_network_header(na_skb);

        na_ip6h = (struct ipv6hdr *)skb_put(na_skb, sizeof(struct ipv6hdr));
        na_ip6h->version = 6;
        na_ip6h->priority = 0;
        na_ip6h->nexthdr = IPPROTO_ICMPV6;
        na_ip6h->hop_limit = 255;
        na_ip6h->payload_len = htons(na_msg_len);
        na_ip6h->daddr = ip6h->saddr;

        in6_pton("fd42:b95b:3fcd:8900:64c6:24fe:1f5a:b55a", -1, src_addr.s6_addr, -1, NULL);
        na_ip6h->saddr = src_addr;

        na = (struct nd_msg *)skb_put(na_skb, na_msg_len);
        memset(na, 0, na_msg_len);
        na->icmph.icmp6_type = ICMPV6_NEIGHBOR_ADVERTISEMENT;
        na->icmph.icmp6_code = 0;
        na->icmph.icmp6_cksum = 0;
        na->target = target_addr;

        na_skb->dev = networkDevice;
        na_skb->protocol = htons(ETH_P_IPV6);

        printk(KERN_INFO "[RX][NDP][%d] Responding to Neighbor Solicitation from %pI6c\n",RX_Count, &ip6h->saddr);

        na->icmph.icmp6_cksum = csum_ipv6_magic(&na_ip6h->saddr, &na_ip6h->daddr,
                                                na_msg_len, IPPROTO_ICMPV6,
                                                csum_partial((char *)na, na_msg_len, 0));

        dev_queue_xmit(na_skb);

        printk(KERN_INFO "[RX][NDP][%d] Sent Neighbor Advertisement\n",RX_Count);
        RX_Count++;
    }
    else if (icmp6->icmp6_type == ICMPV6_NEIGHBOR_ADVERTISEMENT)
    {
        // Manually convert IPv6 address to string
        char src_addr_str[INET6_ADDRSTRLEN];
        inet6_addr_to_str(&ip6h->saddr, src_addr_str, sizeof(src_addr_str));

        printk(KERN_INFO "[RX][NDP][%d] Neighbor Advertisement received from %s on %s\n",RX_Count, src_addr_str, networkDevice->name);
        RX_Count++;
    }

    return NET_RX_SUCCESS;
}
