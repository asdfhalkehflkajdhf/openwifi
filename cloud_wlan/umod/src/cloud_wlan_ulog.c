#include <stdlib.h>  
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>  
#include <linux/netlink.h>  
#include <sys/socket.h>  
#include <strings.h>  
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

//extern s32 h_errno;


#include "cloud_wlan_types.h"
#include "cloud_wlan_nl_out_pub.h"
#include "cloud_wlan_nl_in_pub.h"
#include "cloud_wlan_sqlite.h"
#include "cloud_wlan_ap_com.h"
#include "cloud_wlan_ap_local.h"

#include "cloud_wlan_ulog.h"



/*
struct hostent {
    char *h_name;
    char **h_aliases;
    short h_addrtype;
    short h_length;
    char **h_addr_list;
    #define h_addr h_addr_list[0]
};
*/
static s8 *g_klog_tpye_info[]=
{
	"klog url",
	"klog user",
	"klog unknown"
};

u32 g_cw_debug = 1;

u32 cw_local_log_writeto_file(s8 *str, ...)
{
	FILE *tfp;
	tfp = fopen(CW_AP_ULOG_FILE, "a");
	if ( g_cw_debug && tfp != NULL)
	{
		va_list args;
		va_start(args, str);
		vfprintf(tfp, (char *)str, args);
		va_end(args);

		fclose(tfp);
	}
	return CWLAN_OK;
}

u32 cw_kmod_log_writeto_file(dcma_udp_skb_info_t *buff)
{
	kmod_log_info_t *kmod_log = (kmod_log_info_t *)buff->data;
	FILE *tfp;
	//还需要起个上报线程，完成无端服务器的上报
	if(kmod_log->type == KLOG_URL)
	{
		tfp = fopen(CW_AP_KLOG_URL_FILE, "a");
	}
	else
	{
		tfp = fopen(CW_AP_KLOG_USER_FILE, "a");
	}
	if ( tfp != NULL)
	{
		fprintf(tfp, "%d, %s, %2x:%2x:%2x:%2x:%2x:%2x, %2x:%2x:%2x:%2x:%2x:%2x, %x, %s, %s\n", 
				kmod_log->type, g_klog_tpye_info[kmod_log->type],
				kmod_log->apmac[0],kmod_log->apmac[1],kmod_log->apmac[2],
				kmod_log->apmac[3],kmod_log->apmac[4],kmod_log->apmac[5],
				kmod_log->usermac[0],kmod_log->usermac[1],kmod_log->usermac[2],
				kmod_log->usermac[3],kmod_log->usermac[4],kmod_log->usermac[5],
				kmod_log->userip, 
				ctime((time_t *)&kmod_log->time),
				kmod_log->data
			);
		fclose(tfp);
	}
	return CWLAN_OK;
}

u32 cw_ap_sendto_ac_kmod_log_info(dcma_udp_skb_info_t *buff)
{
	if(g_log.log_mode ==REAL_TIME)
	{
		//cw_kmod_log_writeto_server(buff);
	}
	else
	{
		cw_kmod_log_writeto_file(buff);
	}
	return CWLAN_OK;
}


