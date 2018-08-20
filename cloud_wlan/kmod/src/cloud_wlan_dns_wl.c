#include <linux/module.h>  
#include <linux/netlink.h>  
#include <net/netlink.h>  
#include <net/net_namespace.h>  
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/if_ether.h>

#include "cloud_wlan_nl_out_pub.h"
#include "cloud_wlan_dns_wl.h"

#include "cloud_wlan_main.h"
#include "cloud_wlan_log.h"

dns_white_list_t g_dns_white_list={0,0};
static rwlock_t rwlock;


u32 cloud_wlan_dns_white_list_init(void)
{
	//g_dns_white_list init
	g_dns_white_list.addr = kmalloc(CLOUD_WLAN_DNS_DEFAULT_NUMBER * sizeof(u32),GFP_KERNEL);
	if(g_dns_white_list.addr == NULL)
	{
		return CWLAN_FAIL;
	}
	rwlock_init(&rwlock);
	return CWLAN_OK;
}

u32 cloud_wlan_dns_white_list_update(dns_white_list_t *buf)
{
	u32 i;	
	write_lock_bh(&rwlock);

	memcpy(&g_dns_white_list, buf, sizeof(g_dns_white_list)+ CLOUD_WLAN_DNS_DEFAULT_NUMBER * sizeof(u32));

	printk(" ap cloud mode update_white_list ok:\n");
	for(i=0; i<g_dns_white_list.number; i++)
	{
		printk("[%d] [%x]\n", i, g_dns_white_list.addr[i]);
	}
	write_unlock_bh(&rwlock);
	return CWLAN_OK;
}



u32 cloud_wlan_dns_white_list_find(u32 addr)
{
	u32 i;
	for(i=0; i<g_dns_white_list.number; i++)
	{
		if(addr == g_dns_white_list.addr[i])
		{
			return CWLAN_OK;
		}
	}

	return CWLAN_FAIL;
}

u32 cloud_wlan_dns_white_list_exit(void)
{
	write_lock_bh(&rwlock);
	kfree(g_dns_white_list.addr);
	write_unlock_bh(&rwlock);
	return CWLAN_OK;
}


