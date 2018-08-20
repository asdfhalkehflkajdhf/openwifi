#include <linux/module.h>  
#include <linux/netlink.h>  
#include <net/netlink.h>  
#include <net/net_namespace.h>  
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/if_ether.h>

//#include "cloud_wlan_types.h"
#include "cloud_wlan_nl_out_pub.h"
#include "cloud_wlan_nl_in_pub.h"

#include "cloud_wlan_main.h"
#include "cloud_wlan_session.h"
#include "cloud_wlan_http_pub.h"
#include "cloud_wlan_log.h"
#include "cloud_wlan_user_wl.h"
#include "cloud_wlan_dns_wl.h"

static struct sock *sk; //内核端socket  

/*通信示例函数*/
static void cloud_wlan_nl_get_test(struct nlmsghdr *nlh)
{
    void *payload;  
    struct sk_buff *out_skb;  
    void *out_payload;  
    struct nlmsghdr *out_nlh;  
    int payload_len; // with padding, but ok for echo   
    

	payload = nlmsg_data(nlh);	
	payload_len = nlmsg_len(nlh);  
	printk("payload_len = %d\n", payload_len);  
	printk("Recievid: %s, From: %d\n", (char *)payload, nlh->nlmsg_pid);	

	
	out_skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL); //分配足以存放默认大小的sk_buff  
	if (!out_skb)
		goto failure;  
	//skb, pid, seq, type, len
	out_nlh = nlmsg_put(out_skb, 0, 0, CW_NLMSG_RES_OK, MAX_DATA_PAYLOAD, 0); //填充协议头数据  
	if (!out_nlh)
		goto failure;  
	out_payload = nlmsg_data(out_nlh);	
	// 在响应中加入字符串，以示区别  
	snprintf(out_payload, MAX_DATA_PAYLOAD, "[kernel res info]: GETPID[%d] TYPE [%2X] OK\n", nlh->nlmsg_pid, nlh->nlmsg_type);
	nlmsg_unicast(sk, out_skb, nlh->nlmsg_pid); 
	return;
failure:  
	printk(" failed in fun dataready!\n");  
}
u32 cloud_wlan_nl_debug_off(void)
{

	g_cloud_wlan_debug= CWLAN_CLOSE;
	printk(" ap cloud mode debug close ok\n");
	return CWLAN_OK;
}
u32 cloud_wlan_nl_debug_on(void)
{

	g_cloud_wlan_debug= CWLAN_OPEN;
	printk(" ap cloud mode debug open ok\n");
	return CWLAN_OK;
}
u32 cloud_wlan_nl_cw_off(void)
{

	g_cw_base_cfg.cwlan_sw= CWLAN_OPEN;
	printk(" ap cloud mode close ok\n");
	return CWLAN_OK;
}
u32 cloud_wlan_nl_cw_on(void)
{

	g_cw_base_cfg.cwlan_sw= CWLAN_OPEN;
	printk(" ap cloud mode open ok\n");
	return CWLAN_OK;
}

u32 cloud_wlan_nl_klog_off(void)
{
	g_cw_base_cfg.klog_sw= CWLAN_CLOSE;
	printk(" ap klog mode close ok\n");
	return CWLAN_OK;
}
u32 cloud_wlan_nl_klog_on(void)
{

	g_cw_base_cfg.klog_sw= CWLAN_OPEN;
	printk(" ap klog mode open ok\n");
	return CWLAN_OK;
}
u32 cloud_wlan_nl_cw_update_session_cfg(s8 *buf)
{
	memcpy((void *)&g_cw_base_cfg, (void *)buf, sizeof(g_cw_base_cfg));
	
	printk(" ap cloud mode update_portal_session_cfg ok:\n\n");
	printk(" usage_model: %d\n",g_cw_base_cfg.usage_model);
	printk(" over_time     : %d\n",g_cw_base_cfg.over_time);
	printk(" interval_timer: %d\n",g_cw_base_cfg.interval_timer);

	return CWLAN_OK;
}

u32 cloud_wlan_nl_cw_show_url_white_list(void)
{
	u32 i;
	printk(" ap cloud mode white_list info:\n");
	for(i=0; i<g_dns_white_list.number; i++)
	{
		printk("[%d] [%x]\n", i, g_dns_white_list.addr[i]);
	}
	return CWLAN_OK;
}
u32 cloud_wlan_nl_cw_update_portal_url(s8 *buf)
{
	u32 i;
	memcpy((void *)&g_portal_config.rehttp_conf, (void *)buf, sizeof(reHttp_t));
	
	printk(" ap cloud mode update_portal_info:\n\n %s\n",g_portal_config.rehttp_conf.Location);
	for(i=0; i<CW_LOCATION_URL_IP_MAX; i++)
	{
		printk("[%d] [%x]\n", i, g_portal_config.rehttp_conf.destIp[i]);
	}
	return CWLAN_OK;
}

u32 cloud_wlan_nl_cw_show_online_user(void)
{
	flow_session_show_online_list();
	return CWLAN_OK;
}
u32 cloud_wlan_nl_cw_show_portal_url(void)
{
	u32 i;
	printk(" ap cloud mode portal_url url info:\n %s\n",g_portal_config.rehttp_conf.Location);
	
	printk(" ap cloud mode portal_url ip info:\n");
	for(i=0; i<CW_LOCATION_URL_IP_MAX; i++)
	{
		printk("[%d] [%x]\n", i, g_portal_config.rehttp_conf.destIp[i]);
	}
	return CWLAN_OK;
}
/*
	 内核与用户太通信控制命令总的分之函数，
	 不要在这个函数里边直接添加业务功能，
	 使用switch 分支结构去添加命令和接口函数。
*/
static void cloud_wlan_nl_console_branch(struct sk_buff *skb)  
{  
    struct nlmsghdr *nlh;  

    nlh = nlmsg_hdr(skb);  
    switch(nlh->nlmsg_type)  
    {  
        case CW_NLMSG_RES_OK:  
            break;  
        case CW_NLMSG_GET_TEST:  
			cloud_wlan_nl_get_test(nlh);
            break;
		case CW_NLMSG_SET_USER_PID:
			g_cloud_wlan_nlmsg_pid = *(u32 *)nlmsg_data(nlh);
			break;
		case CW_NLMSG_SET_DEBUG_OFF:
			cloud_wlan_nl_debug_off();
			break;
		case CW_NLMSG_SET_DEBUG_ON:
			cloud_wlan_nl_debug_on();
			break;
		case CW_NLMSG_SET_OFF:
			cloud_wlan_nl_cw_off();
			break;
		case CW_NLMSG_SET_ON:
			cloud_wlan_nl_cw_on();
			break;
		case CW_NLMSG_SET_KLOG_OFF:
			cloud_wlan_nl_klog_off();
			break;
		case CW_NLMSG_SET_KLOG_ON:
			cloud_wlan_nl_klog_on();
			break;
		case CW_NLMSG_UPDATE_URL_WHITE_LIST:
			cloud_wlan_dns_white_list_update((dns_white_list_t *)nlmsg_data(nlh));
			break;
		case CW_NLMSG_USER_WHITE_LIST_ADD:
			cloud_wlan_user_wl_add((user_wl_buf_t *)nlmsg_data(nlh));
			break;
		case CW_NLMSG_USER_WHITE_LIST_DEL:
			cloud_wlan_user_wl_del((user_wl_buf_t *)nlmsg_data(nlh));
		case CW_NLMSG_USER_WHITE_LIST_UPDATE:
			cloud_wlan_user_wl_update((user_wl_buf_t *)nlmsg_data(nlh));
			break;
		case CW_NLMSG_UPDATE_PORTAL:
			cloud_wlan_nl_cw_update_portal_url((s8 *)nlmsg_data(nlh));
			break;
		case CW_NLMSG_UPDATE_SESSION_CFG:
			cloud_wlan_nl_cw_update_session_cfg((s8 *)nlmsg_data(nlh));
			break;
		case CW_NLMSG_DEBUG_SHOW_URL_WHITE_LIST:
			cloud_wlan_nl_cw_show_url_white_list();
			break;
		case CW_NLMSG_DEBUG_SHOW_ONLINE_USER:
			cloud_wlan_nl_cw_show_online_user();
			break;
		case CW_NLMSG_DEBUG_SHOW_PORTAL:
			cloud_wlan_nl_cw_show_portal_url();
			break;
        default:  
            CLOUD_WLAN_DEBUG("Unknow msgtype recieved! [%2x]\n", nlh->nlmsg_type);  
    }  
    return;  
}  
  
u32 cloud_wlan_nl_init(void)
{
    struct netlink_kernel_cfg nlcfg = {  
        .input = cloud_wlan_nl_console_branch,  
    };  
    sk = netlink_kernel_create(&init_net, NETLINK_CWLAN, &nlcfg);  
    if (!sk) {  
		CLOUD_WLAN_DEBUG("cw init netlink_kernel_create fail\n");
    } 
	
	printk("cw init netlink_kernel_create ok\n");
    return CWLAN_OK;  

}
u32 cloud_wlan_nl_exit(void)
{
    netlink_kernel_release(sk); 
	
	printk("cw exit netlink_kernel_create ok\n");
	return CWLAN_OK;
}
/*kernel model only log type upload
one:用户访问日志
tow:用户上下线日志
*/
s32 cloud_wlan_sendto_umod(s32 type, s8 *buff, u32 datalen)
{
	struct sk_buff *out_skb;  
	void *out_payload;	
	struct nlmsghdr *out_nlh;  	

	out_skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL); //分配足以存放默认大小的sk_buff  
	if (!out_skb)
		goto failure;  
	//skb, pid, seq, type, len
	out_nlh = nlmsg_put(out_skb, 0, 0, type, MAX_DATA_PAYLOAD, 0); //填充协议头数据  
	if (!out_nlh)
		goto failure;  
	out_payload = nlmsg_data(out_nlh);	
	// 在响应中加入字符串，以示区别  
	memcpy(out_payload, buff,datalen);
	nlmsg_unicast(sk, out_skb, g_cloud_wlan_nlmsg_pid); 
	return CWLAN_OK;
failure:  
	printk(" failed in fun dataready!\n");	
	return CWLAN_OK;
}

