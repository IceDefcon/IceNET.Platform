/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "receiver.h"
#include "crypto.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static hookControlType hookControl =
{
    .ethernetHeader = NULL,
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
    .replySocketBuffer = NULL,
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

unsigned int receiverHook(void *priv, struct sk_buff *socketBuffer, const struct nf_hook_state *state)
{
    if (!socketBuffer)
    {
        return NF_ACCEPT;
    }

    // if (state->in || state->out)
    // {
    //     pr_info("[L2][ETH] In: %s | Out: %s\n", state->in ? state->in->name : "N/A", state->out ? state->out->name : "N/A");

    //     if (state->in && strcmp(state->in->name, "eth0") == 0)
    //     {
    //         pr_info("[L2][ETH] Packet received on interface: %s\n", state->in->name);
    //     }
    // }

    // if (skb_mac_header_was_set(socketBuffer))
    // {
    //     hookControl.ethernetHeader = eth_hdr(socketBuffer);
    //     if (hookControl.ethernetHeader)
    //     {
    //         pr_info("[L2][ETH] Src MAC: %pM -> Dst MAC: %pM | Proto: 0x%04x\n",
    //                 hookControl.ethernetHeader->h_source,
    //                 hookControl.ethernetHeader->h_dest,
    //                 ntohs(hookControl.ethernetHeader->h_proto));
    //     }
    // }

    hookControl.ipHeader = ip_hdr(socketBuffer);
    if (!hookControl.ipHeader)
    {
        return NF_ACCEPT;
    }

    // pr_info("[L3][IP] Src IP: %pI4 -> Dst IP: %pI4 | Proto: %d\n", &hookControl.ipHeader->saddr, &hookControl.ipHeader->daddr, hookControl.ipHeader->protocol);

    if (hookControl.ipHeader->protocol == IPPROTO_UDP)
    {
        hookControl.udpHeader = udp_hdr(socketBuffer);
        if (!hookControl.udpHeader)
        {
            return NF_ACCEPT;
        }

        hookControl.destPort = ntohs(hookControl.udpHeader->dest);
        if (hookControl.destPort != UDP_PORT)
        {
            return NF_ACCEPT;
        }

        if (hookControl.ipHeader->daddr != htonl(BROADCAST_IP))
        {
            return NF_ACCEPT;
        }

        hookControl.payload = (u8 *)hookControl.udpHeader + sizeof(struct udphdr);
        hookControl.payloadLength = ntohs(hookControl.udpHeader->len) - sizeof(struct udphdr);

        // pr_info("[L4][UDP] Src Port: %d -> Dst Port: %d\n", ntohs(hookControl.udpHeader->source), hookControl.destPort);

        if (hookControl.payloadLength > 0 && skb_tail_pointer(socketBuffer) >= hookControl.payload + hookControl.payloadLength)
        {
            // pr_info("[L7][DATA][UDP] Encrypted Payload: %.32s\n", hookControl.payload);
            if (aesDecrypt(hookControl.payload, hookControl.payloadLength, hookControl.aesKey, NULL) == 0)
            {
                if('I' == hookControl.payload[0] && 'c' == hookControl.payload[1] && 'e' == hookControl.payload[2])
                {
                    pr_info("[L4][UDP] Src Port: %d -> Dst Port: %d\n", ntohs(hookControl.udpHeader->source), hookControl.destPort);
                    pr_info("[L7][DATA][UDP] Decrypted Payload: %.32s\n", hookControl.payload);
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
    else if (hookControl.ipHeader->protocol == IPPROTO_TCP)
    {
        hookControl.tcpHeader = (struct tcphdr *)((__u32 *)hookControl.ipHeader + hookControl.ipHeader->ihl);
        if (!hookControl.tcpHeader)
        {
            return NF_ACCEPT;
        }

        hookControl.destPort = ntohs(hookControl.tcpHeader->dest);
        if (hookControl.destPort != TCP_PORT)
        {
            return NF_ACCEPT;
        }

        hookControl.payload = (u8 *)hookControl.tcpHeader + (hookControl.tcpHeader->doff * 4);
        hookControl.payloadLength = ntohs(hookControl.ipHeader->tot_len) - (hookControl.ipHeader->ihl * 4) - (hookControl.tcpHeader->doff * 4);

        // pr_info("[L4][TCP] Src Port: %d -> Dst Port: %d\n", ntohs(hookControl.tcpHeader->source), ntohs(hookControl.tcpHeader->dest));

        if (hookControl.payloadLength > 0 && skb_tail_pointer(socketBuffer) >= hookControl.payload + hookControl.payloadLength)
        {
            // pr_info("[L7][DATA][TCP] Encrypted Payload: %.32s\n", hookControl.payload);
            if (aesDecrypt(hookControl.payload, hookControl.payloadLength, hookControl.aesKey, NULL) == 0)
            {
                if('I' == hookControl.payload[0] && 'c' == hookControl.payload[1] && 'e' == hookControl.payload[2])
                {
                    pr_info("[L4][TCP] Src Port: %d -> Dst Port: %d\n", ntohs(hookControl.tcpHeader->source), ntohs(hookControl.tcpHeader->dest));
                    pr_info("[L7][DATA][TCP] Decrypted Payload: %.32s\n", hookControl.payload);
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

    if (ntohs(arpRequest.arpHeader->ar_op) != ARPOP_REQUEST)
    {
        if (ntohs(arpRequest.arpHeader->ar_op) == ARPOP_REPLY)
        {
            // ARP Reply received
            pr_info("[RX][ARP] ARP Reply received from %pI4\n", &arpRequest.senderIp);

            // Ensure we're correctly parsing the ARP header to extract the MAC address
            arpRequest.arpPtr = (unsigned char *)(arpRequest.arpHeader + 1); // Move pointer past ARP header

            // Extract Sender MAC Address (SHA) and print it
            arpRequest.senderMacAddress = arpRequest.arpPtr;
            pr_info("[RX][ARP] Sender MAC Address (from ARP Reply): %pM\n", arpRequest.senderMacAddress);

            // Extract Sender IP Address (SPA)
            arpRequest.senderIpAddress = arpRequest.senderMacAddress + arpRequest.arpHeader->ar_hln;
            pr_info("[RX][ARP] Sender IP Address: %pI4\n", arpRequest.senderIpAddress);

            // If needed, you can also extract the Target MAC Address (THA) and Target IP Address (TPA)
            arpRequest.targetMacAddress = arpRequest.senderIpAddress + arpRequest.arpHeader->ar_pln;
            arpRequest.targetIpAddress = arpRequest.targetMacAddress + arpRequest.arpHeader->ar_hln;

            // Print Target MAC Address (THA) and Target IP Address (TPA)
            pr_info("[RX][ARP] Target MAC Address: %pM\n", arpRequest.targetMacAddress);
            pr_info("[RX][ARP] Target IP Address: %pI4\n", arpRequest.targetIpAddress);
        }
        return NET_RX_SUCCESS;
    }

    arpRequest.arpPtr = (unsigned char *)(arpRequest.arpHeader + 1);
    arpRequest.senderMacAddress = arpRequest.arpPtr;
    arpRequest.senderIpAddress = arpRequest.senderMacAddress + arpRequest.arpHeader->ar_hln;
    arpRequest.targetMacAddress = arpRequest.senderIpAddress + arpRequest.arpHeader->ar_pln;
    arpRequest.targetIpAddress = arpRequest.targetMacAddress + arpRequest.arpHeader->ar_hln;

    // pr_info("[RX][ARP] SHA (Sender MAC) = %pM\n", arpRequest.senderMacAddress);
    // pr_info("[RX][ARP] SPA (Sender IP) = %pI4\n", arpRequest.senderIpAddress);
    // pr_info("[RX][ARP] THA (Target MAC) = %pM\n", arpRequest.targetMacAddress);
    // pr_info("[RX][ARP] TPA (Target IP) = %pI4\n", arpRequest.targetIpAddress);

    memcpy(&arpRequest.senderIp, arpRequest.senderIpAddress, 4);   // Sender IP
    memcpy(&arpRequest.targetIp, arpRequest.targetIpAddress, 4);   // Target IP
    // pr_info("[RX][ARP] Sender IP = %pI4, Target IP = %pI4\n", &arpRequest.senderIp, &arpRequest.targetIp);

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

    arpRequest.replySocketBuffer = alloc_skb(ETH_HLEN + arpRequest.arpHeaderLength, GFP_ATOMIC);
    if (!arpRequest.replySocketBuffer)
    {
        pr_err("[RX][ARP] Failed to allocate replySocketBuffer for ARP reply\n");
        return NET_RX_DROP;
    }
    // pr_info("[RX][ARP] Successfully allocated replySocketBuffer for ARP reply: replySocketBuffer=%p\n", arpRequest.replySocketBuffer);

    skb_reserve(arpRequest.replySocketBuffer, ETH_HLEN);               // Reserve space for Ethernet header
    skb_put(arpRequest.replySocketBuffer, arpRequest.arpHeaderLength);                // Make room for ARP header/data
    arpRequest.ethernetHeader = (struct ethhdr *)skb_push(arpRequest.replySocketBuffer, ETH_HLEN);            // Push Ethernet header

    // pr_info("[RX][ARP] replySocketBuffer headroom: %d, tailroom: %d\n", skb_headroom(arpRequest.replySocketBuffer), skb_tailroom(arpRequest.replySocketBuffer));

    arpRequest.replySocketBuffer->dev = arpRequest.networkDevice;
    arpRequest.replySocketBuffer->protocol = htons(ETH_P_ARP);
    skb_reset_mac_header(arpRequest.replySocketBuffer);
    skb_reset_network_header(arpRequest.replySocketBuffer);

    // pr_info("[RX][ARP] MAC header set at: %p\n", skb_mac_header(arpRequest.replySocketBuffer));

    arpRequest.ethernetHeader = (struct ethhdr *)skb_mac_header(arpRequest.replySocketBuffer);
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

    dev_queue_xmit(arpRequest.replySocketBuffer);

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
