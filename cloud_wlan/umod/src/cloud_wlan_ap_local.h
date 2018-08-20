#ifndef CLOUD_WLAN_AP_LOCAL_H_
#define CLOUD_WLAN_AP_LOCAL_H_

#define CWLAN_AP_CFG_DB "/etc/cloudwlan/cw_ap_cfg.db"
#define CWLAN_AP_CFG_TABLE "t_config_base"
#define CWLAN_AP_LOG "t_log"
#define CWLAN_AP_URL_WHITE_TABLE "t_url_white_list"
#define CWLAN_AP_USER_WHITE_TABLE "t_user_white_list"
#define CWLAN_AP_PORTAL_TABLE "t_portal_list"
#define CWLAN_AP_ONLINE_DB_TABLE "t_online_socket"


struct t_config_base{
	u32 usage_model;
	u32 cwlan_sw;
	u32 time_online;
	u32 interval_timer;
	u8 wan_eth_name[32];
	u8 wan_eth_mac[6];
};

struct t_online_socket{
	u8 server_addr[32];
	u32 server_port;
};

struct t_user_white_list{
	u8 mac[6];
	u32 portal_id;
	u32 time_online;
};

struct t_log{
	u32 ulog_sw;
	u32 klog_sw;
	u32 log_mode;
	u8 server_addr[20];
	u32 server_port;
	u8 server_path[128];
	u8 server_user[32];
	u8 server_password[64];
};



extern struct t_config_base g_config_base;
extern struct t_online_socket g_online_socket;
extern struct t_log g_log;




/*++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


typedef struct cpu_info
{
	u32 user; 
	u32 nice;
	u32 system;
	u32 idle;
	u32 iowait;
	u32 irp;
	u32 softirp;
	u32 stealstolen;
	u32 guest;
}cpu_info_t;

extern 	u32 cw_get_ap_local_info(ap_local_info_t *ap_info);
#endif
