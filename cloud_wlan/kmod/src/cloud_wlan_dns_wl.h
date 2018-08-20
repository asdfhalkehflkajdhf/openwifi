#ifndef CLOUD_WLAN_DNS_WL_H_
#define CLOUD_WLAN_DNS_WL_H_
#include "cloud_wlan_nl_in_pub.h"

extern dns_white_list_t g_dns_white_list;


extern u32 cloud_wlan_dns_white_list_init(void);
extern u32 cloud_wlan_dns_white_list_update(dns_white_list_t *buf);
extern u32 cloud_wlan_dns_white_list_find(u32 addr);
extern u32 cloud_wlan_dns_white_list_exit(void);


#endif

