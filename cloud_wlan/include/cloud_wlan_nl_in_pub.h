#ifndef CLOUD_WLAN_NL_IN_PUB_H_
#define CLOUD_WLAN_NL_IN_PUB_H_

#define CLOUD_WLAN_DNS_DEFAULT_NUMBER 1024

typedef struct dns_white_list
{
	u32 number;
	u32 *addr;
}dns_white_list_t;

typedef struct user_wl_buf
{
	u32 number;
	u8 *buff;
}user_wl_buf_t;

enum klog_type
{
	KLOG_URL,
	KLOG_USER,
	KLOG_MAX
};

/*所有数据通信结构*/
typedef struct kmod_log_info
{
	u32 size;
	u32 type;
	u32 userip;
	u8 usermac[6];
	u8 apmac[6];
	u64 time;
	u8 data[];
}kmod_log_info_t;


#endif

