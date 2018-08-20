#include <linux/module.h>  
#include <linux/netlink.h>  
#include <net/netlink.h>  
#include <net/net_namespace.h>  
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/if_ether.h>

#include "cloud_wlan_nl_out_pub.h"
#include "cloud_wlan_nl_in_pub.h"

#include "cloud_wlan_main.h"
#include "cloud_wlan_session.h"
#include "cloud_wlan_http_pub.h"
#include "cloud_wlan_log.h"


s8 *g_klog_descrip[]={"UP", "DOWN", "UPDATE"};

u32 cloud_wlan_klog_url(struct sk_buff *skb, pkt_tuple_info_t *quintuple_info)
{
	u32 ret;
	u32 bufflen;
	kmod_log_info_t *buff;
	http_t url_log;
	
	bufflen = sizeof(kmod_log_info_t)+HTTP_HOST_LEN;

	ret = cloud_wlan_http_skb_parse_request(skb, &url_log);
	if(ret == CWLAN_FAIL)
	{
		return CWLAN_FAIL;
	}
	
	buff = kmalloc(bufflen, GFP_KERNEL);
	if(buff == NULL)
	{
		return CWLAN_FAIL;
	}
	buff->size = bufflen;
	buff->userip = quintuple_info->saddr;
	memcpy(buff->usermac, quintuple_info->smac, 6);
	memcpy(buff->data, url_log.host, HTTP_HOST_LEN);
	
	cloud_wlan_sendto_umod(CW_NLMSG_PUT_KLOG_INFO, (s8 *)buff, bufflen);

	kfree(buff);
	
	return CWLAN_OK;
}

u32 cloud_wlan_klog_user(struct sk_buff *skb, pkt_tuple_info_t *quintuple_info, s8 *descrip)
{
	u32 bufflen;
	kmod_log_info_t *buff;
	
	bufflen = sizeof(kmod_log_info_t)+strlen(descrip)+1;

	buff = kmalloc(bufflen, GFP_KERNEL);
	if(buff == NULL)
	{
		return CWLAN_FAIL;
	}
	buff->size = bufflen;
	buff->userip = quintuple_info->saddr;
	memcpy(buff->usermac, quintuple_info->smac, 6);
	memcpy(buff->data, descrip, strlen(descrip)+1);
	
	cloud_wlan_sendto_umod(CW_NLMSG_PUT_KLOG_INFO, (s8 *)buff, bufflen);

	kfree(buff);
	
	return CWLAN_OK;
}

u32 cloud_wlan_generate_klog_main(struct sk_buff *skb, pkt_tuple_info_t *quintuple_info, u32 log_type, s8 *descrip)
{
	struct tcphdr *tcphdr;
	tcphdr = tcp_hdr(skb);

	if(CWLAN_OPEN != g_cw_base_cfg.klog_sw )
	{
		return CWLAN_FAIL;
	}

	switch(log_type)
	{
		case KLOG_URL:
			if( ntohs(tcphdr->dest) == PROTO_HTTP)
			{
				cloud_wlan_klog_url(skb, quintuple_info);
			}
			break;
		case KLOG_USER:
			cloud_wlan_klog_user(skb, quintuple_info, descrip);
			break;
		default:
			break;
	}
	
	return CWLAN_OK;
}
