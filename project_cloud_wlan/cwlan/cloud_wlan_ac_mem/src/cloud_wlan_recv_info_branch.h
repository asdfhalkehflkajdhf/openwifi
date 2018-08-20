#ifndef CLOUD_WLAN_RECV_INFO_BRANCH_H_
#define CLOUD_WLAN_RECV_INFO_BRANCH_H_


enum ap_web_auth_modt
{
	AP_OPEN_AUTH,
	AP_ADMIN_AUTH,
};


typedef struct dcma_pthread_info
{
	struct sockaddr *client_info;
	void *data;	
}dcma_pthread_info_t;

typedef struct cwlan_cmd_sockt
{
	u8 apmac[6];
	u32 size;
	s8 data[0];
}cwlan_cmd_sockt_t;

extern u32 g_ap_auth_mode;

extern u32 cw_recv_info_dispose_pthread_cfg_init();
extern u32 cw_recv_info_dispose_pthread_cfg_exit();

extern void cw_recv_info_branch(dcma_udp_skb_info_t *buff, u32 len, struct sockaddr *client_info);


#endif
