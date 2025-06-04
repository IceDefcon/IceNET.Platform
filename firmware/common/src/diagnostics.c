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
        pr_info(" - %pI4\n", &host->ip);
    }

    spin_unlock_bh(&active_host_lock);
}

void addActiveHost(__be32 ip)
{
    // Check if this IP is already in the active list
    struct active_host *host;
    bool found = false;

    spin_lock_bh(&active_host_lock);
    list_for_each_entry(host, &active_host_list, list)
    {
        if (host->ip == ip)
        {
            found = true;
            break;
        }
    }

    // If not found, add to list
    if (!found)
    {
        host = kmalloc(sizeof(*host), GFP_ATOMIC);
        if (host)
        {
            host->ip = ip;
            list_add_tail(&host->list, &active_host_list);
            pr_info("[DISCOVERY] New active host: %pI4\n", &ip);
        }
    }

    spin_unlock_bh(&active_host_lock);
}

void removeActiveHost(__be32 ip)
{
    struct active_host *host, *tmp;

    spin_lock_bh(&active_host_lock);
    list_for_each_entry_safe(host, tmp, &active_host_list, list)
    {
        if (host->ip == ip)
        {
            list_del(&host->list);
            kfree(host);
            pr_info("[DISCOVERY] Removed active host: %pI4\n", &ip);
            break;  // IPs are unique in list, so break after removing
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
