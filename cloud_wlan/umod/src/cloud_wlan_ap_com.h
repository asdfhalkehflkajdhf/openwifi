#ifndef CLOUD_WLAN_AP_COM_H_
#define CLOUD_WLAN_AP_COM_H_



#define DNS_CONFIG_FILE "/etc/config/network"




struct pthread_id
{
	pthread_t communicat_id;
	pthread_t recv_k_id;
	pthread_t update_id;
};
extern struct pthread_id g_pthread;

typedef struct cw_nl_info
{
	s32 sockfd;  
	struct nlmsghdr *nlh;  
	struct sockaddr_nl src_addr;
	struct sockaddr_nl dst_addr;	
	struct msghdr msg;
}cw_nl_info_t;

extern s32 cloud_wlan_nl_cfg_init(void);
extern s32 cloud_wlan_nl_close(void);
extern s32 cloud_wlan_sendto_kmod(int type, s8 *buff, u32 datalen);
extern s32 cloud_wlan_sendto_kmod_ok(int type, s8 *buff, u32 datalen);
extern void *cloud_wlan_recv_kmod_info(void *param);

#endif
