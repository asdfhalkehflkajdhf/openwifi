#ifndef CLOUD_WLAN_LOG_H_
#define CLOUD_WLAN_LOG_H_

#include "cloud_wlan_session.h"


enum klog_descrip
{
	KLOG_DESCRIP_USER_UP,
	KLOG_DESCRIP_USER_DOWN,
	KLOG_DESCRIP_URL_NULL,
	KLOG_DESCRIP_MAX
};

extern s8 *g_klog_descrip[];


extern u32 cloud_wlan_generate_klog_main(struct sk_buff *skb, pkt_tuple_info_t *quintuple_info, u32 log_type, s8 *descrip);


#endif

