/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "diagnostics.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LIST_HEAD(active_host_list);
spinlock_t active_host_lock;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void initActiveHostList(void)
{
    INIT_LIST_HEAD(&active_host_list);
    spin_lock_init(&active_host_lock);
    pr_info("[INIT] Active host list and lock initialized.\n");
}

void printActiveHosts(void)
{
    struct active_host *host;

    spin_lock_bh(&active_host_lock);

    pr_info("[ACTIVE HOSTS LIST]:\n");
    list_for_each_entry(host, &active_host_list, list)
    {
        pr_info(" - IP: %pI4, MAC: %pM\n", &host->ip, host->mac);
    }

    spin_unlock_bh(&active_host_lock);
}

void addActiveHost(__be32 ip, const unsigned char *mac)
{
    struct active_host *host;
    bool found = false;

    spin_lock_bh(&active_host_lock);
    list_for_each_entry(host, &active_host_list, list)
    {
        if (host->ip == ip && ether_addr_equal(host->mac, mac))
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        host = kmalloc(sizeof(*host), GFP_ATOMIC);
        if (host)
        {
            host->ip = ip;
            memcpy(host->mac, mac, ETH_ALEN);
            list_add_tail(&host->list, &active_host_list);
            pr_info("[DISCOVERY] New active host: %pI4 (%pM)\n", &ip, mac);
        }
    }

    spin_unlock_bh(&active_host_lock);
}

void removeActiveHost(__be32 ip, const unsigned char *mac)
{
    struct active_host *host, *tmp;

    spin_lock_bh(&active_host_lock);
    list_for_each_entry_safe(host, tmp, &active_host_list, list)
    {
        if (host->ip == ip && ether_addr_equal(host->mac, mac))
        {
            list_del(&host->list);
            kfree(host);
            pr_info("[DISCOVERY] Removed active host: %pI4 (%pM)\n", &ip, mac);
            break;
        }
    }
    spin_unlock_bh(&active_host_lock);
}

void cleanupActiveHostList(void)
{
    struct active_host *host, *tmp;

    spin_lock_bh(&active_host_lock);

    list_for_each_entry_safe(host, tmp, &active_host_list, list)
    {
        list_del(&host->list);  // Remove from list
        kfree(host);            // Free the memory
    }

    spin_unlock_bh(&active_host_lock);

    pr_info("[CLEANUP] Active host list cleared.\n");
}
