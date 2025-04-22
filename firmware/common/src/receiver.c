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

    // pr_info("[ARP] Received ARP request, socketBuffer=%p, networkDevice=%s\n", socketBuffer, arpRequest.networkDevice->name);

    arpRequest.arpHeader = arp_hdr(socketBuffer);
    if (!arpRequest.arpHeader)
    {
        pr_err("[ARP] Failed to extract ARP header from socketBuffer: %p\n", socketBuffer);
        return NET_RX_DROP;
    }
    // pr_info("[ARP] Extracted ARP header: arpHeader=%p, operation=%u\n", arpRequest.arpHeader, ntohs(arpRequest.arpHeader->ar_op));

    if (ntohs(arpRequest.arpHeader->ar_op) != ARPOP_REQUEST)
    {
        // pr_info("[ARP] Not an ARP request, skipping\n");
        return NET_RX_SUCCESS;
    }

    arpRequest.arpPtr = (unsigned char *)(arpRequest.arpHeader + 1);
    arpRequest.senderMacAddress = arpRequest.arpPtr;
    arpRequest.senderIpAddress = arpRequest.senderMacAddress + arpRequest.arpHeader->ar_hln;
    arpRequest.targetMacAddress = arpRequest.senderIpAddress + arpRequest.arpHeader->ar_pln;
    arpRequest.targetIpAddress = arpRequest.targetMacAddress + arpRequest.arpHeader->ar_hln;

    // pr_info("[ARP] SHA (Sender MAC) = %pM\n", arpRequest.senderMacAddress);
    // pr_info("[ARP] SPA (Sender IP) = %pI4\n", arpRequest.senderIpAddress);
    // pr_info("[ARP] THA (Target MAC) = %pM\n", arpRequest.targetMacAddress);
    // pr_info("[ARP] TPA (Target IP) = %pI4\n", arpRequest.targetIpAddress);

    memcpy(&arpRequest.senderIp, arpRequest.senderIpAddress, 4);   // Sender IP
    memcpy(&arpRequest.targetIp, arpRequest.targetIpAddress, 4);   // Target IP
    // pr_info("[ARP] Sender IP = %pI4, Target IP = %pI4\n", &arpRequest.senderIp, &arpRequest.targetIp);

    if (arpRequest.targetIp != arpRequest.ourIp || arpRequest.senderIp != arpRequest.allowedSenderIp)
    {
        // pr_info("[ARP] Ignoring request from %pI4 for %pI4 (Expected IPs: sender=%pI4, target=%pI4)\n", &arpRequest.senderIp, &arpRequest.targetIp, &arpRequest.allowedSenderIp, &arpRequest.ourIp);
        return NET_RX_SUCCESS;
    }

    // pr_info("[ARP] Responding to ARP request from %pI4 asking for %pI4\n", &arpRequest.senderIp, &arpRequest.targetIp);

    memcpy(arpRequest.ourMac, arpRequest.networkDevice->dev_addr, ETH_ALEN); // Our MAC
    // pr_info("[ARP] Our MAC address: %pM\n", arpRequest.ourMac);

    arpRequest.arpHeaderLength = sizeof(struct arphdr) + 2 * (ETH_ALEN + 4); // hln=6, pln=4
    // pr_info("[ARP] ARP header length calculated: %d\n", arpRequest.arpHeaderLength);

    arpRequest.replySocketBuffer = alloc_skb(ETH_HLEN + arpRequest.arpHeaderLength, GFP_ATOMIC);
    if (!arpRequest.replySocketBuffer)
    {
        pr_err("[ARP] Failed to allocate replySocketBuffer for ARP reply\n");
        return NET_RX_DROP;
    }
    // pr_info("[ARP] Successfully allocated replySocketBuffer for ARP reply: replySocketBuffer=%p\n", arpRequest.replySocketBuffer);

    skb_reserve(arpRequest.replySocketBuffer, ETH_HLEN);               // Reserve space for Ethernet header
    skb_put(arpRequest.replySocketBuffer, arpRequest.arpHeaderLength);                // Make room for ARP header/data
    arpRequest.ethernetHeader = (struct ethhdr *)skb_push(arpRequest.replySocketBuffer, ETH_HLEN);            // Push Ethernet header

    // pr_info("[ARP] replySocketBuffer headroom: %d, tailroom: %d\n", skb_headroom(arpRequest.replySocketBuffer), skb_tailroom(arpRequest.replySocketBuffer));

    arpRequest.replySocketBuffer->dev = arpRequest.networkDevice;
    arpRequest.replySocketBuffer->protocol = htons(ETH_P_ARP);
    skb_reset_mac_header(arpRequest.replySocketBuffer);
    skb_reset_network_header(arpRequest.replySocketBuffer);

    // pr_info("[ARP] MAC header set at: %p\n", skb_mac_header(arpRequest.replySocketBuffer));

    arpRequest.ethernetHeader = (struct ethhdr *)skb_mac_header(arpRequest.replySocketBuffer);
    memcpy(arpRequest.ethernetHeader->h_dest, arpRequest.senderMacAddress, ETH_ALEN);         // Dest: sender MAC
    memcpy(arpRequest.ethernetHeader->h_source, arpRequest.ourMac, ETH_ALEN);   // Src: our MAC
    arpRequest.ethernetHeader->h_proto = htons(ETH_P_ARP);            // Protocol type

    // pr_info("[ARP] Ethernet header: dst=%pM, src=%pM, proto=0x%04x\n", arpRequest.ethernetHeader->h_dest, arpRequest.ethernetHeader->h_source, ntohs(arpRequest.ethernetHeader->h_proto));

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

    // pr_info("[ARP] ARP header: hrd=%u, pro=%u, hln=%d, pln=%d, op=%u\n",
    //     ntohs(arpRequest.arpHeader->ar_hrd), ntohs(arpRequest.arpHeader->ar_pro),
    //     arpRequest.arpHeader->ar_hln, arpRequest.arpHeader->ar_pln, ntohs(arpRequest.arpHeader->ar_op));

    // Fill ARP payload
    arpRequest.arpReplyPtr = (unsigned char *)(arpRequest.arpHeader + 1);
    memcpy(arpRequest.arpReplyPtr, arpRequest.ourMac, ETH_ALEN);                // Sender MAC
    memcpy(arpRequest.arpReplyPtr + ETH_ALEN, &arpRequest.ourIp, 4);            // Sender IP
    memcpy(arpRequest.arpReplyPtr + ETH_ALEN + 4, arpRequest.senderMacAddress, ETH_ALEN);     // Target MAC
    memcpy(arpRequest.arpReplyPtr + 2 * ETH_ALEN + 4, arpRequest.senderIpAddress, 4);        // Target IP

    dev_queue_xmit(arpRequest.replySocketBuffer);

    pr_info("[ARP] Reply Successfully Sent -> %pI4\n", &arpRequest.senderIp);
    pr_info("[ARP] Reply Payload -> senderMac[%pM] senderIp[%pI4] targetMac[%pM] targetIp[%pI4]\n",
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
