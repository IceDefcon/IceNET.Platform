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
#define SRC_PORT     12000
#define DST_PORT     54000
#define PAYLOAD_LEN  32

#define IFACE_NAME  "wlp2s0"
#define LISTEN_PORT 54000
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

// Declare the RSA public key handle
struct crypto_akcipher *rsaPublicKey;

// 2048-bit RSA Public Key
static const u8 rsa_modulus[] = {
    0xBB, 0x95, 0x95, 0x99, 0x12, 0x95, 0x64, 0x0B, 0x5F, 0x9C, 0x6D, 0xBC, 0xF1, 0x35, 0xD1, 0xBC,
    0xA4, 0x4F, 0xB1, 0x17, 0xA9, 0x34, 0xD7, 0x46, 0x8B, 0xF6, 0x6A, 0xD4, 0x4E, 0xF6, 0x25, 0x18,
    0x60, 0x1F, 0xAE, 0x28, 0xC3, 0xE5, 0x78, 0x35, 0x39, 0xF1, 0x1E, 0x8F, 0x46, 0x67, 0xD3, 0x1A,
    0x3B, 0x37, 0xEC, 0xAE, 0x82, 0x70, 0xE5, 0x27, 0x3F, 0xA0, 0x1D, 0xC3, 0xD5, 0x98, 0x6B, 0x3A,
    0xE2, 0x5A, 0x91, 0xF3, 0x92, 0x31, 0x7F, 0x99, 0x28, 0x6A, 0xB7, 0x23, 0xF3, 0xE6, 0x3F, 0x1F,
    0x75, 0x7B, 0x29, 0x56, 0x22, 0x4A, 0x4F, 0x56, 0x1A, 0x56, 0xF1, 0x6A, 0x9F, 0xD6, 0x72, 0x77,
    0xE9, 0x5E, 0x35, 0x2B, 0x24, 0x0A, 0xA6, 0x98, 0xE5, 0x29, 0x56, 0xD4, 0x28, 0x62, 0xF5, 0x2E,
    0x5F, 0xE4, 0x5D, 0xF4, 0x51, 0x96, 0x60, 0x17, 0xC3, 0x70, 0x5E, 0xEC, 0xC4, 0x97, 0xB9, 0xC4,
    0xA7, 0xAE, 0x4C, 0xC1, 0xB9, 0x19, 0x52, 0x34, 0x39, 0x37, 0x5E, 0xBB, 0x29, 0x1A, 0x74, 0x91,
    0xF0, 0xB0, 0x0C, 0x43, 0x1D, 0x29, 0x92, 0x45, 0x56, 0x28, 0x79, 0x34, 0x90, 0xB1, 0xB9, 0xA3,
    0xE2, 0xE7, 0x25, 0xD2, 0x7A, 0x90, 0x98, 0xD0, 0xD4, 0x0D, 0x6B, 0xA9, 0xDA, 0xC5, 0xE6, 0x17,
    0x73, 0x56, 0xA0, 0xD5, 0x1C, 0xDB, 0x5E, 0x2F, 0x18, 0x14, 0x0E, 0x70, 0x94, 0x56, 0x99, 0x5F,
    0x70, 0xD5, 0x9D, 0x28, 0x10, 0x0E, 0x21, 0x1B, 0xE7, 0xED, 0x39, 0xF2, 0xFD, 0x25, 0x74, 0x63,
    0x9F, 0xB4, 0x6C, 0xD3, 0x9A, 0x03, 0x98, 0x53, 0x45, 0x66, 0xD9, 0xFE, 0xA6, 0x36, 0xB6, 0x83,
    0xF3, 0x1A, 0x9C, 0x07, 0x8B, 0x41, 0xC2, 0xAF, 0xD8, 0x05, 0xF8, 0xD6, 0x70, 0x83, 0xCE, 0xD7,
    0xB8, 0xDD, 0x7C, 0x04, 0xA5, 0xED, 0xB9, 0x8C, 0x72, 0xAB, 0xA9, 0xBD, 0x84, 0xEE, 0xE0, 0x42
};

// Public exponent (usually 0x10001, commonly used exponent for RSA)
static const u8 rsa_exponent[] = { 0x01, 0x00, 0x01 };  // 65537 in hex (0x10001)

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

int init_rsa_key(void)
{
    int err;
    struct akcipher_request *req;

    // Allocate the RSA public key cipher
    rsaPublicKey = crypto_alloc_akcipher("rsa", 0, 0);
    if (IS_ERR(rsaPublicKey)) {
        pr_err("Failed to allocate RSA public key cipher\n");
        return PTR_ERR(rsaPublicKey);
    }

    // Allocate the request structure for the crypto API
    req = akcipher_request_alloc(rsaPublicKey, GFP_KERNEL);
    if (!req) {
        pr_err("Failed to allocate RSA request structure\n");
        crypto_free_akcipher(rsaPublicKey);
        return -ENOMEM;
    }

    // Set the RSA public key (modulus and exponent)
    err = crypto_akcipher_set_pub_key(rsaPublicKey, rsa_modulus, sizeof(rsa_modulus));
    if (err) {
        pr_err("Failed to set RSA public key\n");
        akcipher_request_free(req);
        crypto_free_akcipher(rsaPublicKey);
        return err;
    }

    pr_info("RSA public key initialized successfully\n");

    // Free the request structure as it's no longer needed
    akcipher_request_free(req);

    return 0;
}

int udpTransmissionRSA(void)
{
    int err;
    u8 *rsaEncryptedPayload;  // Pointer for RSA-encrypted payload
    size_t encryptedLength = 0;

    // Dynamically allocate memory for the RSA-encrypted payload
    rsaEncryptedPayload = kmalloc(2048, GFP_ATOMIC);  // Allocate 2048 bytes
    if (!rsaEncryptedPayload)
    {
        pr_err("[TX][UDP] Memory allocation failed for RSA-encrypted payload\n");
        return -ENOMEM;
    }

    transferControl.transmissionLength = ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr) + RSA_MAX_ENCRYPT_SIZE; // Adjust for RSA encryption

    // [L2] Data Link Layer: Set destination to broadcast MAC
    memset(transferControl.destinationMAC, 0xFF, ETH_ALEN);

    // [L3] Network Layer
    transferControl.source_IP = in_aton(SRC_IP);
    transferControl.dest_IP = in_aton(DST_IP);

    // [L2] Memory allocation for socket buffer
    transferControl.socketBuffer = alloc_skb(transferControl.transmissionLength + NET_IP_ALIGN, GFP_ATOMIC);
    if (!transferControl.socketBuffer)
    {
        kfree(rsaEncryptedPayload);  // Free the dynamically allocated memory before returning
        return -ENOMEM;
    }

    skb_reserve(transferControl.socketBuffer, NET_IP_ALIGN);
    skb_reserve(transferControl.socketBuffer, ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr));

    init_rsa_key();

    // [L7] Application Layer: Encrypt payload using RSA
    // In this case, rsaEncrypt is used to encrypt the payload with RSA public key
    err = rsaEncrypt((u8 *)"Ice.Marek.IceNET.Technology.x-86", PAYLOAD_LEN, rsaEncryptedPayload, rsaPublicKey);
    if (err < 0)
    {
        pr_err("[TX][UDP][L6] RSA encryption failed\n");
        kfree(rsaEncryptedPayload);  // Free the dynamically allocated memory before returning
        kfree_skb(transferControl.socketBuffer);
        return -EIO;
    }

    encryptedLength = err;  // Length of the RSA-encrypted data

    print_hex_dump(KERN_INFO, "[TX][UDP][L7] RSA Encrypted Payload: ", DUMP_PREFIX_OFFSET, 16, 1, rsaEncryptedPayload, encryptedLength, false);

    // [L4] Transport Layer: UDP header setup
    skb_push(transferControl.socketBuffer, sizeof(struct udphdr));
    transferControl.udpHeader = (struct udphdr *)transferControl.socketBuffer->data;
    transferControl.udpHeader->source = htons(SRC_PORT);
    transferControl.udpHeader->dest = htons(DST_PORT);
    transferControl.udpHeader->len = htons(sizeof(struct udphdr) + encryptedLength); // Use encryptedLength here
    transferControl.udpHeader->check = 0;

    // [L3] Network Layer: IP header setup
    skb_push(transferControl.socketBuffer, sizeof(struct iphdr));
    transferControl.ipHeader = (struct iphdr *)transferControl.socketBuffer->data;
    transferControl.ipHeader->version = 4;
    transferControl.ipHeader->ihl = 5;
    transferControl.ipHeader->tos = 0;
    transferControl.ipHeader->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + encryptedLength); // Adjust for encrypted length
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

    // [L7] Place the RSA-encrypted payload into the UDP packet (after headers)
    skb_put(transferControl.socketBuffer, encryptedLength);
    memcpy(transferControl.socketBuffer->data + ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr), rsaEncryptedPayload, encryptedLength);

    if (dev_queue_xmit(transferControl.socketBuffer) < 0)
    {
        pr_err("[TX][UDP][L1] Failed to transmit UDP packet\n");
        kfree(rsaEncryptedPayload);  // Free memory before returning
        kfree_skb(transferControl.socketBuffer);
        return -EIO;
    }

    pr_info("[TX][UDP][L1] RSA encrypted UDP packet sent to broadcast\n");

    // Free dynamically allocated memory after successful transmission
    kfree(rsaEncryptedPayload);

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
        if (dport != LISTEN_PORT)
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
            if (aesDecrypt(payload, payload_len, aes_key, NULL) == 0)
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
            if (aesDecrypt(payload, payload_len, aes_key, NULL) == 0)
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
    pr_info("[INIT][NET] Master Control Network Initialisation\n");

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
