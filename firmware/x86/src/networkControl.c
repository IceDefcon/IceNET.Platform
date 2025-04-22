/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "networkControl.h"
#include "transmitter.h"
#include "receiver.h"
#include "crypto.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct nf_hook_ops netFilterHook;
static struct packet_type arpPacket;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int configureNetworkDevice(void)
{
    networkControlType* networkControl = getNetworkController();
    networkControl->iface_name = "wlp2s0";

    networkControl->networkDevice = dev_get_by_name(&init_net, networkControl->iface_name);
    if (!networkControl->networkDevice)
    {
        pr_err("[ERNO][NET] Device %s not found\n", networkControl->iface_name);
        return -ENODEV;
    }

    if (!(networkControl->networkDevice->flags & IFF_UP))
    {
        pr_err("[ERNO][NET] Interface %s is down\n", networkControl->iface_name);
        dev_put(networkControl->networkDevice);
        return -ENETDOWN;
    }

    if (!is_valid_ether_addr(networkControl->networkDevice->dev_addr))
    {
        pr_err("[ERNO][NET] Invalid MAC address on %s\n", networkControl->iface_name);
        dev_put(networkControl->networkDevice);
        return -EINVAL;
    }

    return 0;
}

int networkInit(void)
{
    int ret = configureNetworkDevice();

    pr_info("[INIT][NET] Master Control Network Initialisation\n");

    if(-ENODEV == ret || -ENETDOWN == ret || -EINVAL == ret)
    {
        pr_err("[ERNO][NET] Huston, we have a problem -> %d\n", ret);
        return ret;
    }

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
        pr_info("[INIT][NET] Sniffing TCP[port: %d] and UDP[port: %d]\n",TCP_PORT, UDP_PORT);
    }

    /**
     * [L2] ARP is not an IP protocol it
     * is Layer 2 protocol and cannot
     * be hooked by the net filter
     */
    arpPacket.type = htons(ETH_P_ARP);
    arpPacket.func = arpReceive;
    dev_add_pack(&arpPacket);

    return 0;
}

void networkDestroy(void)
{
    networkControlType* networkControl = getNetworkController();

    nf_unregister_net_hook(&init_net, &netFilterHook);

    if (networkControl->networkDevice)
    {
        dev_remove_pack(&arpPacket);
        dev_put(networkControl->networkDevice);
        networkControl->networkDevice = NULL;
    }

    pr_info("[TX][DESTROY] IceNET Master Controller Module Unloaded\n");
}
