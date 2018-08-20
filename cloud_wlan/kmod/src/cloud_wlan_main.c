#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <linux/if.h>
#include <linux/socket.h>

#include <net/arp.h>


//#include "cloud_wlan_types.h"
#include "cloud_wlan_nl_out_pub.h"
#include "cloud_wlan_nl_in_pub.h"

#include "cloud_wlan_main.h"
#include "cloud_wlan_session.h"
#include "cloud_wlan_http_pub.h"
#include "cloud_wlan_log.h"
#include "cloud_wlan_dns_wl.h"
#include "cloud_wlan_user_wl.h"

rwlock_t g_rwlock;

u32 g_cloud_wlan_debug = CWLAN_CLOSE;
u32 g_cloud_wlan_nlmsg_pid = 0;

/* IP Hooks */
/* After promisc drops, checksum checks. */
#define NF_IP_PRE_ROUTING	0
/* If the packet is destined for this box. */
#define NF_IP_LOCAL_IN		1
/* If the packet is destined for another interface. */
#define NF_IP_FORWARD		2
/* Packets coming from a local process. */
#define NF_IP_LOCAL_OUT		3
/* Packets about to hit the wire. */
#define NF_IP_POST_ROUTING	4
#define NF_IP_NUMHOOKS		5

/* 用于注册我们的函数的数据结构 */ 
struct nf_hook_ops g_cwlan_in_hook_prer; 
struct nf_hook_ops g_cwlan_out_hook_prer; 

u32 cloud_wlan_get_quintuple(struct sk_buff *skb, pkt_tuple_info_t *quintuple_info)
{
	struct iphdr *iphdr;
	struct tcphdr *tcphdr;
	struct ethhdr *eth_hdr;
	
	struct dst_entry *dst = skb_dst(skb);
	struct net_device *dev = dst->dev;
	struct neighbour *neigh;

	
	iphdr = ip_hdr(skb);
	tcphdr = tcp_hdr(skb);
	quintuple_info->saddr = iphdr->saddr;
	quintuple_info->daddr = iphdr->daddr;
	quintuple_info->source = tcphdr->source;
	quintuple_info->dest = tcphdr->dest;
	quintuple_info->protocol = iphdr->protocol;

	//eth_hdr = eth_hdr(skb);
	neigh = __ipv4_neigh_lookup_noref(dev, iphdr->daddr);
	if(neigh != NULL)
	{
		if(neigh->hh.hh_len > ETH_HLEN)
		{
			eth_hdr = (struct ethhdr *)neigh->hh.hh_data;
		}
		else
		{
			eth_hdr = (struct ethhdr *)(neigh->hh.hh_data+2);
		}
	}
	memcpy(quintuple_info->smac, eth_hdr->h_source, 6); 
	memcpy(quintuple_info->dmac, eth_hdr->h_dest, 6); 

	
	return CWLAN_OK;
}
/******************************************************************************* 
功能:过滤条件判断
 -------------------------------------------------------------------------------
参数:	pdesc 原报文信息
-------------------------------------------------------------------------------
返回值:	CWLAN_FAIL	
			CWLAN_OK	forwar处理
*******************************************************************************/
u32 cloud_wlan_packet_transparent_forward(struct sk_buff *skb)
{
	//struct ethhdr *ethhdr;
	struct iphdr *iphdr;
	struct tcphdr *tcphdr;

	//struct eth8021hdr *ethhdr_8021;
	/*
	1、cloud wlan 的一个全局开关
		判断下是否为ap到外网报文
	*/

	if( g_cw_base_cfg.cwlan_sw== CWLAN_CLOSE|| skb == NULL || skb->dev == NULL )
	{
		return CWLAN_OK;
	}
	/*
	ethhdr_8021 = (struct eth8021hdr *)eth_hdr(skb);
	if(ethhdr_8021->ethhdr.h_proto != htons(ETH_P_IP) && ethhdr_8021->ethhdr.h_proto != htons(ETH_P_8021Q))
	{
		CLOUD_WLAN_DEBUG("not ETH_P_IP or ETH_P_8021Q\n");
		return CWLAN_OK;
	}
	
	if(ethhdr_8021->ethhdr.h_proto == htons(ETH_P_8021Q) && ethhdr_8021->proto != htons(ETH_P_IP))
	{
		CLOUD_WLAN_DEBUG("is ETH_P_8021Q but not ETH_P_IP\n");
		return CWLAN_OK;
	}
*/
	if( memcmp("br-lan", skb->dev->name, 6) )
	{
		return CWLAN_OK;
	}
	
	iphdr = ip_hdr(skb);
	tcphdr = tcp_hdr(skb);

	//包过滤目的端口为snmp,dns端口号则报文透传
	/* DNS DHCP CAPWAP SNMP Needs to be forwarded, you can add remove or make a corresponding interface*/
	if( iphdr->protocol == IPPROTO_ICMP )
	{
		return CWLAN_OK;
	}
	switch(ntohs(tcphdr->dest))
	{
		case PROTO_DNS:
		case PROTO_DHCP67:
		case PROTO_DHCP68:
		case PROTO_CAPWAP_C:
		case PROTO_CAPWAP_D:
		case PROTO_SNMP1:
		case PROTO_SNMP2:
		case PROTO_SSH:
		//case PROTO_HTTP:
		//case PROTO_HTTPS:
		//case PROTO_HTTP2:
			return CWLAN_OK;
		default:
			break;
	}

	return CWLAN_FAIL;

}

/* 注册的hook函数的实现 */ 
u32 cwlan_in_hook_prer(u32 hooknum,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       u32 (*okfn)(struct sk_buff *))
{
	u32 ret;
	u32 i;
	//struct ethhdr *ethhdr;
	struct iphdr *iphdr;
	struct tcphdr *tcphdr;
	pkt_tuple_info_t cw_quintuple_info;


	//过滤报文
	ret = cloud_wlan_packet_transparent_forward(skb);
	if(ret == CWLAN_OK )
	{
		goto  accept;
	}
	
	iphdr = ip_hdr(skb);
	tcphdr = tcp_hdr(skb);
	/*
	2、目的地址是在dns白名单里的直接过
	*/
	for(i=0; i<CW_LOCATION_URL_IP_MAX; i++)
	{
		if(ntohl(iphdr->daddr) == g_portal_config.rehttp_conf.destIp[i])
		{
			goto  accept;
		}
	}
	if(cloud_wlan_dns_white_list_find((u32)ntohl(iphdr->daddr)) == CWLAN_OK)
	{
		goto  accept;
	}
	cloud_wlan_get_quintuple(skb, &cw_quintuple_info);
	/*
	2.1、目的地址是在user白名单里的直接过
	*/

	if(cloud_wlan_user_white_list_find(cw_quintuple_info.smac) == CWLAN_OK)
	{
		goto  accept;
	}
	
	/*
	3、查下在线用户列表，返回认证状态和一些信息，判断是否通过
		不存在，则新建一个用户节点
	*/
	ret = flow_session_match_online_list(&cw_quintuple_info);
	switch (ret)
	{
		case CW_USER_ONLINE_STATE_UP:
			CLOUD_WLAN_DEBUG("\t\tmatch accept.\n\n");
/*在这里生成url访问日志*/
			cloud_wlan_generate_klog_main(skb, &cw_quintuple_info, KLOG_URL, NULL);
			goto  accept;
		case CW_USER_ONLINE_STATE_DOWN:
			CLOUD_WLAN_DEBUG("\t\tmatch protal.\n\n");
			break;
		default :
			CLOUD_WLAN_DEBUG("\t\tmatch drop.\n\n");
			goto  drop;
	}

	//read_unlock_bh(&g_rwlock);
	//目前只能处理80重定向，443直接丢包
	if( iphdr->protocol == IPPROTO_TCP && ntohs(tcphdr->dest) == PROTO_HTTP)
	{
		//源包直接丢掉
		reply_http_redirector(skb);
	}

drop:
	return NF_DROP;
accept:
	return NF_ACCEPT;
}

/* 注册的hook函数的实现 */ 
u32 cwlan_out_hook_prer(u32 hooknum,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       u32 (*okfn)(struct sk_buff *))
{
	u32 ret;
	u32 i;
	

	struct iphdr *iphdr;
	struct tcphdr *tcphdr;
	pkt_tuple_info_t quintuple_info;
	s8 *buff=NULL;
/*云wlan未开启或是报文为null直接过*/
	if( g_cw_base_cfg.cwlan_sw == CWLAN_CLOSE|| skb == NULL || skb->dev == NULL )
	{
		goto  accept;
	}
	
	iphdr = ip_hdr(skb);
	tcphdr = tcp_hdr(skb);
	/*不是http报文直接过*/
	if(	(ntohs(tcphdr->source) != PROTO_HTTP && ntohs(tcphdr->source)!= PROTO_HTTP2))
	{
		goto  accept;
	}
	/*
	2、目的地址是在白名单里的直接过
	*/
	for(i=0; i<CW_LOCATION_URL_IP_MAX; i++)
	{
		if(ntohl(iphdr->saddr) == g_portal_config.rehttp_conf.destIp[i] 
			&& tcphdr->syn != 1)
		{
			//看是是不是认证成功
			buff = (s8 *)tcphdr + tcphdr->doff * 4;
			ret = cloud_wlan_http_skb_parse_reply(buff,NULL);
			if(ret == CWLAN_OK)
			{
				cloud_wlan_get_quintuple(skb, &quintuple_info);
	

				
				CLOUD_WLAN_DEBUG("out [%x][%x][%d][%d]    [%2x][%2x][%2x][%2x][%2x][%2x]	[%2x][%2x][%2x][%2x][%2x][%2x]\n",
					quintuple_info.saddr,quintuple_info.daddr,
					quintuple_info.protocol,tcphdr->dest,

					quintuple_info.smac[0],quintuple_info.smac[1],
					quintuple_info.smac[2],quintuple_info.smac[3],
					quintuple_info.smac[4],quintuple_info.smac[5],

					quintuple_info.dmac[0],quintuple_info.dmac[1],
					quintuple_info.dmac[2],quintuple_info.dmac[3],
					quintuple_info.dmac[4],quintuple_info.dmac[5]
					);
				//注意，之前需要进行对在线时长解析
				flow_session_up_node(&quintuple_info, 1800);
			}
			break;
		}
	}
	
accept:
	return NF_ACCEPT;
}

u32 kmod_hook_init(void)
{
	/* 该钩子对应的处理函数 */
	g_cwlan_in_hook_prer.hook = (nf_hookfn *)cwlan_in_hook_prer;
	/* 使用IPv4的第一个hook */
	g_cwlan_in_hook_prer.hooknum  = NF_IP_PRE_ROUTING;
	g_cwlan_in_hook_prer.pf       = PF_INET; 
	g_cwlan_in_hook_prer.priority = NF_IP_PRI_FIRST;   /* 让我们的函数首先执行 */

	/*将用户自己定义的钩子注册到内核中*/ 
	nf_register_hook(&g_cwlan_in_hook_prer);

	
	/* 该钩子对应的处理函数 */
	g_cwlan_out_hook_prer.hook = (nf_hookfn *)cwlan_out_hook_prer;
	/* 使用IPv4的第一个hook */
	g_cwlan_out_hook_prer.hooknum  = NF_IP_POST_ROUTING;
	g_cwlan_out_hook_prer.pf       = PF_INET; 
	//g_cwlan_out_hook_prer.priority = NF_IP_PRI_FIRST;   /* 让我们的函数首先执行 */

	/*将用户自己定义的钩子注册到内核中*/ 
	nf_register_hook(&g_cwlan_out_hook_prer);
	return CWLAN_OK;
}
u32 kmod_hook_exit(void)
{
	//将用户自己定义的钩子从内核中删除 
    nf_unregister_hook(&g_cwlan_in_hook_prer);
	//将用户自己定义的钩子从内核中删除 
    nf_unregister_hook(&g_cwlan_out_hook_prer);
	return CWLAN_OK;
}
extern u32 cloud_wlan_nl_init(void);
extern u32 cloud_wlan_nl_exit(void);

/* cloud_module_init —- 初始化函数，
当模块装载时被调用，如果成功装载返回0 否则返回非0值 */
s32 __init cloud_module_init(void)
{
	cloud_wlan_nl_init();


	//g_user_white_list init
	cloud_wlan_user_white_list_init();
	//g_dns_white_list init
	cloud_wlan_dns_white_list_init();

	flow_sesison_init();
	//重定向配置初始化默认值
	reply_http_redirector_init();

	rwlock_init(&g_rwlock);

	kmod_hook_init();
	
	printk("cw init cloud_wlan kmod finish\n");

	return 0;
}
/*cloud_module_exit —- 退出函数，
当模块卸载时被调用*/
void __exit cloud_module_exit(void)
{
	g_cw_base_cfg.cwlan_sw = CWLAN_CLOSE;
	kmod_hook_exit();
	
	cloud_wlan_nl_exit();

	flow_session_exit();
	
	printk("cw exit cloud_wlan finish!\n");

}
module_init(cloud_module_init);
module_exit(cloud_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("cloud_lzc");
