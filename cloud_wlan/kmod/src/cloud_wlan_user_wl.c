#include <linux/module.h>  
#include <linux/netlink.h>  
#include <net/netlink.h>  
#include <net/net_namespace.h>  
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/if_ether.h>

#include "cloud_wlan_nl_out_pub.h"
#include "cloud_wlan_user_wl.h"

#include "cloud_wlan_main.h"
#include "cloud_wlan_log.h"

user_white_list_t g_user_white_list;
static rwlock_t rwlock;


u32 cloud_wlan_user_white_list_init(void)
{
	//g_user_white_list init
	INIT_LIST_HEAD(&g_user_white_list.list);
	rwlock_init(&rwlock);
	return CWLAN_OK;
}

u32 cloud_wlan_user_white_list_add(u8 *mac)
{
	user_white_list_t *node;
	user_white_list_t *temp_node;
	user_white_list_t *new_node;

	write_lock_bh(&rwlock);

    list_for_each_entry_safe(node, temp_node, &g_user_white_list.list, list)
	{
	
		if( memcmp(node->mac, mac, 6) == 0)
		{
			write_unlock_bh(&rwlock);
			return CWLAN_OK;
		}
	}

	CLOUD_WLAN_DEBUG("\tnot find user wl and add node.\n");

	new_node = kmalloc(sizeof(user_white_list_t),GFP_KERNEL);
	if(new_node != NULL)
	{
		memcpy(new_node->mac, mac, 6);
		
		list_add(&new_node->list,&g_user_white_list.list);
	}
	
	write_unlock_bh(&rwlock);
	return CWLAN_OK;
}

//更新用户白名单
u32 cloud_wlan_user_white_list_update(u8 *mac)
{
	user_white_list_t *node;
	user_white_list_t *temp_node;

	write_lock_bh(&rwlock);

    list_for_each_entry_safe(node, temp_node, &g_user_white_list.list, list)
	{
	
		if( memcmp(node->mac, mac, 6) == 0)
		{
			write_unlock_bh(&rwlock);
			return CWLAN_OK;
		}
	}

	CLOUD_WLAN_DEBUG("\tnot find user wl and update del node.\n");
	//没有找到说明没有了需要删除
	list_del(&node->list);
	kfree(node);	
	write_unlock_bh(&rwlock);
	return CWLAN_OK;
}

u32 cloud_wlan_user_white_list_del(u8 *mac)
{
	user_white_list_t *node;
	user_white_list_t *temp_node;
	write_lock_bh(&rwlock);

	list_for_each_entry_safe(node, temp_node, &g_user_white_list.list, list)
	{
		if( memcmp(node->mac, mac, 6) != 0)
		{
			continue;
		}
		list_del(&node->list);
		kfree(node);
	}	
	write_unlock_bh(&rwlock);

	return CWLAN_OK;
}
u32 cloud_wlan_user_wl_add(user_wl_buf_t *buff)
{
	u32 i;
	for(i = 0; i<buff->number; i++)
	{
		cloud_wlan_user_white_list_add(buff->buff+i*6);
	}
	return CWLAN_OK;
}
u32 cloud_wlan_user_wl_del(user_wl_buf_t *buff)
{
	u32 i;
	for(i = 0; i<buff->number; i++)
	{
		cloud_wlan_user_white_list_del(buff->buff+i*6);
	}
	return CWLAN_OK;

}
u32 cloud_wlan_user_wl_update(user_wl_buf_t *buff)
{
	u32 i;
	for(i = 0; i<buff->number; i++)
	{
		cloud_wlan_user_white_list_update(buff->buff+i*6);
	}
	return CWLAN_OK;

}

u32 cloud_wlan_user_white_list_find(u8 *mac)
{
	user_white_list_t *node;
	user_white_list_t *temp_node;

	list_for_each_entry_safe(node, temp_node, &g_user_white_list.list, list)
	{
		if( memcmp(node->mac, mac, 6) == 0)
		{
			return CWLAN_OK;
		}
	}	

	return CWLAN_FAIL;
}

u32 cloud_wlan_user_white_list_exit(void)
{
	user_white_list_t *node;
	user_white_list_t *temp_node;
	write_lock_bh(&rwlock);
	list_for_each_entry_safe(node, temp_node, &g_user_white_list.list, list)
	{
		list_del(&node->list);
		kfree(node);
	}	
	write_unlock_bh(&rwlock);
	return CWLAN_OK;
}


