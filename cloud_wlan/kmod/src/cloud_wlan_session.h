#ifndef CLOUD_WLAN_SESSION_H_
#define CLOUD_WLAN_SESSION_H_


#define CWLAN_FLOW_SESSION_HASH_LEN 200


typedef struct pkt_tuple_info
{
	u8 smac[6];				//惟一确定一个用户
	u8 dmac[6];
	u32 saddr;
	u32 daddr;
	u16 source;
	u16 dest;
	u8 protocol;
}pkt_tuple_info_t;

typedef struct cwlan_flow_session_node
{
	struct list_head list;
	pkt_tuple_info_t original_tuple;
	u32 status;				//结点在线状态up down
	u64 add_time;		//结点新建时间
	u64 last_access_time;		//最后一次访问时间
	u64 available_time;		//可用访问时间
	u64 flow_B;				//使用的流量以字节计
	s32 fragment_num;		//收到两个分片包要回一个ack
}cwlan_flow_session_node_t;

typedef struct cwlan_flow_session_hash
{
	struct list_head h_head;	
	rwlock_t rwlock;
}cwlan_flow_session_hash_t;

extern cwlan_flow_session_hash_t *g_flow_session_hash;

/*以后可以支持用户或远程配置*/
extern cwlan_base_cfg_t g_cw_base_cfg;

extern u32 flow_session_match_online_list(pkt_tuple_info_t *quintuple_info);
extern u32 flow_session_show_online_list(void);
extern u32 flow_session_up_node(pkt_tuple_info_t *quintuple_info, int over_time);
extern u32 flow_session_down_node(pkt_tuple_info_t *quintuple_info);

//extern u32 flow_session_tcp_fragment_clr(pkt_tuple_info_t *quintuple_info);
//extern u32 flow_session_tcp_fragment_inc(pkt_tuple_info_t *quintuple_info);
extern u32 flow_sesison_init(void);
extern u32 flow_session_exit(void);


#endif /* READ_PA_INI_H_ */
