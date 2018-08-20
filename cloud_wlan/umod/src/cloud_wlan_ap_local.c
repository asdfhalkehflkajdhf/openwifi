#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <linux/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <stdlib.h>  
#include <linux/netlink.h>  
#include <strings.h>  
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <sys/stat.h>
#include <stdint.h>
#include <netdb.h>
#include <linux/sockios.h>  


#include "cloud_wlan_types.h"
#include "cloud_wlan_nl_out_pub.h"
#include "cloud_wlan_nl_in_pub.h"
#include "cloud_wlan_sqlite.h"
#include "cloud_wlan_ap_com.h"
#include "cloud_wlan_ap_local.h"


struct t_config_base g_config_base;
struct t_online_socket g_online_socket;
struct t_log g_log;



#define ETHTOOL_GLINK        0x0000000a /* Get link status (ethtool_value) */

struct ethtool_value
{
    u32    cmd;
    u32    data;
};

u32 get_interface_info(s8 *ifname, u32 *ip, s8 *mac)     
{   
    struct ifreq req;     
    struct sockaddr_in *host;    
    struct ethtool_value edata;
	u32 sockfd;
    if(-1 == (sockfd = socket(PF_INET, SOCK_STREAM, 0)))   
    {   
        perror( "socket" );     
        return -1;   
	}
  
    memset(&req, 0, sizeof(struct ifreq));     
    strcpy(req.ifr_ifrn.ifrn_name, ifname); 
    edata.cmd = ETHTOOL_GLINK;
again_get:
    edata.data = -1;
    req.ifr_data = (s8 *)&edata;
	ioctl(sockfd, SIOCETHTOOL, &req);
	if( edata.data != 1)
	{
		cw_local_log_writeto_file("interface %s is down again_get\n");  
		sleep(2);
		goto again_get;
	}
    ioctl(sockfd, SIOCGIFADDR, &req);
    host = (struct sockaddr_in*)&req.ifr_ifru.ifru_addr;    
	if(ip != NULL){
		(*ip) = host->sin_addr.s_addr;
	}
	if(mac != NULL){
		ioctl(sockfd, SIOCGIFHWADDR, &req);
		memcpy(mac, req.ifr_ifru.ifru_hwaddr.sa_data, 6);
	}
    close(sockfd);     

    return 0;
} 


u32 get_mem_info (ap_local_info_t *ap_info)
{
	FILE *fd;          
	char buff[256];   
	char tmp[30];

	//system("cat /proc/meminfo | sed -n '1,2p' > /tmp/apinfo.txt"); 

	fd = fopen ("/proc/meminfo", "r"); 

	fgets (buff, sizeof(buff), fd); 
	sscanf (buff, "%s %u", tmp, &ap_info->mem_total); 
	fgets (buff, sizeof(buff), fd); 
	sscanf (buff, "%s %u", tmp, &ap_info->mem_free); 
	//printf ("%u %u\n", tmp, ap_info->mem_total, ap_info->mem_free); 

	fclose(fd);
	return CWLAN_OK;
}


u32 get_run_time(ap_local_info_t *ap_info)
{
	FILE *fd;          
	char buff[256];   
	char tmp[30];

	fd = fopen ("/proc/uptime", "r"); 

	fgets(buff, sizeof(buff), fd); 
	sscanf(buff, "%llu", &ap_info->run_time); 
	//printf ("%u %u\n", tmp, ap_info->mem_total, ap_info->mem_free); 

	fclose(fd);
	return CWLAN_OK;
}

u32 cal_cpu_rate (cpu_info_t *o, cpu_info_t *n) 
{   
	u64 one, two;    
	u64 idle, sd;
	u64 totalcputime;
	u32 cpu_use = 0;   

	one = (u64) (o->user + o->nice + o->system +o->idle+o->iowait+o->irp+ o->softirp+ o->stealstolen+ o->guest);//???(??+???+??+??)??????od
	two = (u64) (n->user + n->nice + n->system +n->idle+n->iowait+n->irp+ n->softirp+ n->stealstolen+ n->guest);//???(??+???+??+??)??????od
	  
	idle = (u64) (n->idle - o->idle); 

	totalcputime = two-one;
	if(totalcputime != 0)
		cpu_use = idle*100/totalcputime;
	else
		cpu_use = 0;
	return cpu_use;
}

u32 get_cpu_info (cpu_info_t *cpust)
{   
	FILE *fd;         
	int n;            
	char buff[256]; 
	char tmp[30];
	cpu_info_t *cpu_occupt;
	cpu_occupt=cpust;
	         
	//system("cat /proc/stat | sed -n '1,1p' > /tmp/apinfo.txt;cat /proc/stat | sed -n '1,1p' >> /tmp/apinfo.txt"); 

	fd = fopen ("/proc/stat", "r"); 
	fgets (buff, sizeof(buff), fd);

	sscanf (buff, "%s %u %u %u %u %u %u %u %u %u",
		tmp, &cpu_occupt->user, &cpu_occupt->nice,&cpu_occupt->system, &cpu_occupt->idle, &cpu_occupt->iowait,
		&cpu_occupt->irp, &cpu_occupt->softirp, &cpu_occupt->stealstolen, &cpu_occupt->guest );
	/*
	printf ("%s %u %u %u %u %u %u %u %u %u\n",
		tmp, cpu_occupt->user, cpu_occupt->nice,cpu_occupt->system, cpu_occupt->idle, cpu_occupt->iowait,
		cpu_occupt->irp, cpu_occupt->softirp, cpu_occupt->stealstolen, cpu_occupt->guest );
	*/

	fclose(fd);
	return CWLAN_OK;
}

u32 cw_get_ap_local_info(ap_local_info_t *ap_info)
{
    cpu_info_t cpu_stat1;
    cpu_info_t cpu_stat2;
    
	get_mem_info(ap_info);
	get_run_time(ap_info);

	get_cpu_info((cpu_info_t *)&cpu_stat1);
	sleep(1);
	get_cpu_info((cpu_info_t *)&cpu_stat2);
	ap_info->cpu_idle_rate = cal_cpu_rate ((cpu_info_t *)&cpu_stat1, (cpu_info_t *)&cpu_stat2);
	return CWLAN_OK;
}

#define SET_P_NULL(p) if((p) != NULL){*(p)='\0';}

s8* get_Second_Level_Domain(s8 *dest)
{
	s8 * p = NULL;
	s8 * e = NULL;
	p = strchr(dest, ':');
	if(p == NULL)
	{
	/*url?? 1
	www.chinanews.com/gj/2014/04-16/6069354.shtml
	*/
		e = strchr(dest, '/');
		SET_P_NULL(e);
		return dest;
	}
	
	if(strncmp(p, "://", 3) != 0)
	{
	/*url?? 2
	www.chinanews.com:8080/gj/2014/04-16/6069354.shtml
	*/
		SET_P_NULL(p);
		return dest;
	}

	p = p+3;
	e = strchr(p, ':');
	if(e == NULL)
	{
	/*url?? 3
	http://www.chinanews.com/gj/2014/04-16/6069354.shtml
	*/
		e = strchr(p, 47);
		SET_P_NULL(e);
		return p;
	}
	else
	{
	/*url?? 4
	http://www.chinanews.com:8080/gj/2014/04-16/6069354.shtml
	*/
		SET_P_NULL(e);
		return p;
	}

}

u32 cw_ap_local_get_db_config_base(sqlite3 * db)
{
	sqlite3_res res;
	s8 *buf=NULL;
	u32 ret;
	
  	snprintf(g_cw_sql, G_CW_SQL_BUF_LEN, "select * from %s;", CWLAN_AP_CFG_TABLE);
	ret = sqlite3_exec_get_res(db, g_cw_sql, &res);
	if(ret != CWLAN_OK)
	{
		printf("INIT:sqlite3_exec_get_res[%s]: %s\n",g_cw_sql, sqlite3_errmsg(g_cw_db));  
		exit(1);
	}
	sqlite3_get_u32(&res, 1, "usage_model", &g_config_base.usage_model);
	printf("INIT:cwlan usage_model          %d\n", g_config_base.usage_model);

	sqlite3_get_u32(&res, 1, "cwlan_sw", &g_config_base.cwlan_sw);
	printf("INIT:cwlan cwlan_sw             %d\n", g_config_base.cwlan_sw);
	sqlite3_get_u32(&res, 1, "time_online", &g_config_base.time_online);
	printf("INIT:cwlan time_online          %d\n", g_config_base.time_online);
	sqlite3_get_u32(&res, 1, "interval_timer", &g_config_base.interval_timer);
	printf("INIT:cwlan interval_timer       %d\n", g_config_base.interval_timer);
	sqlite3_get_u8(&res, 1, "wan_eth_name", (u8 **)&buf);
	memcpy(g_config_base.wan_eth_name,buf, 8);
	printf("INIT:cwlan wan_eth_name      %d\n", g_config_base.wan_eth_name);

	sqlite3_exec_free_res(&res);

	return CWLAN_OK;
}
s32 cw_ap_local_db_portal_list_update(void)
{
	s32 ret;
	u32 i;
	dns_white_list_t *portal_white_list;
	reHttp_t *portal_cfg;
	sqlite3_res res;
	s8 *url = NULL;

	portal_cfg = malloc(sizeof(reHttp_t));
	portal_white_list = malloc(sizeof(dns_white_list_t));

	if(portal_white_list == NULL || portal_cfg == NULL)
	{
		cw_local_log_writeto_file("cw_ap_local_update_portal_wl fail\n");
		return CWLAN_FAIL;
	}
	
	memset((u8 *)portal_cfg, 0, sizeof(reHttp_t));
	memset((u8 *)portal_white_list, 0, sizeof(dns_white_list_t));

	//目前只能重定向一个页面
	snprintf(g_cw_sql, G_CW_SQL_BUF_LEN, "select url from %s;", CWLAN_AP_PORTAL_TABLE);
	
	//data_len = sqlite3_binary_read(g_cw_db, g_cw_sql, &decryt, 0);
	//DES_Act(out, decryt, data_len,g_des_key, DES_KEY_LEN, DECRYPT);
	ret = sqlite3_exec_get_res(g_cw_db, g_cw_sql, &res);
	if(ret != CWLAN_OK)
	{
		cw_local_log_writeto_file("sqlite3_exec_get_res[%s]: %s\n",g_cw_sql, sqlite3_errmsg(g_cw_db));  
		return CWLAN_FAIL;
	}
	for(i=1; i<2; i++)
	{
		sqlite3_get_s8(&res, i, "url", &url);
		if(url != NULL)
		{
			cw_local_log_writeto_file("INIT:cwlan portal_list url: %s\n",url);
			ret = cw_get_url_dns(url, portal_white_list);
			if(ret == CWLAN_FAIL)
			{
				cw_local_log_writeto_file("cw_get_url_dns fail\n");
				goto local_exit;
			}
		}
	}

	memcpy((u8 *)portal_cfg->destIp, (u8 *)portal_white_list->addr, sizeof(u32)*( CW_LOCATION_URL_IP_MAX-1));
	portal_cfg->destPort = CW_LOCATION_PORT;
	memcpy(portal_cfg->Location, url, CW_LOCATION_URL_DATA_LEN);
	cloud_wlan_sendto_kmod(CW_NLMSG_UPDATE_PORTAL, (s8 *)portal_cfg, sizeof(reHttp_t));
	
local_exit:
	sqlite3_exec_free_res(&res);
	free(portal_white_list);
	free(portal_cfg);
	return CWLAN_OK;
}

s32 cw_get_url_dns(s8 *url, dns_white_list_t *dns_tmp)
{
	s32 i = 0;
	s8 t[128];
	s8 *temp;
	struct hostent *host = NULL;
	
	if(url == NULL || dns_tmp == NULL)
	{
	    return CWLAN_FAIL;
	}

	strcpy(t, url);
	temp = get_Second_Level_Domain(t);

	host = gethostbyname2(temp, AF_INET);
	if (host == NULL)
	{
	/*这里nslookup失败的时候可以直接删除d b中的表项*/
	    cw_local_log_writeto_file("gethostbyname err: %s\n",url);
	    return CWLAN_FAIL;
	}

	for (i=0; host->h_addr_list[i]!= NULL; i++)
	{
		if(dns_tmp->number >= CLOUD_WLAN_DNS_DEFAULT_NUMBER)
		{
			continue;
		}
		dns_tmp->addr[dns_tmp->number]= *(u32 *)host->h_addr_list[i];

	    cw_local_log_writeto_file("ip: %x \n",*(u32 *)host->h_addr_list[i]);
		dns_tmp->number++;
	}

	return CWLAN_OK;
}

s32 cw_ap_local_db_url_wl_update(dns_white_list_t *dns_white_list)
{
	u32 ret;
	u32 i;
	s8 *url = NULL;
	sqlite3_res res;

	snprintf(g_cw_sql, G_CW_SQL_BUF_LEN, "select * from %s;", CWLAN_AP_URL_WHITE_TABLE);
	ret = sqlite3_exec_get_res(g_cw_db, g_cw_sql, &res);
	if(ret != CWLAN_OK)
	{
		cw_local_log_writeto_file("sqlite3_exec_get_res[%s]: %s\n",g_cw_sql, sqlite3_errmsg(g_cw_db));  
		return CWLAN_FAIL;
	}
	for(i=1; i<=res.nrow; i++)
	{
		sqlite3_get_s8(&res, i, "url", &url);
		if(url != NULL)
		{
			cw_local_log_writeto_file("INIT:cwlan white_list url: %s\n",url);
			cw_get_url_dns(url, dns_white_list);
		}
	}
	sqlite3_exec_free_res(&res);
	
	return CWLAN_OK;
}

s32 cw_ap_local_db_dns_update(sqlite3 * db)
{
	dns_white_list_t dns_white_list = {0,NULL};
	sqlite3_res res;
	s8 *buf=NULL;
	u32 ret;

	s8 *url;
	u32 i;
	u32 ip;

	//初始化dns白名最大单个数
	dns_white_list.addr = malloc(sizeof(u32)*CLOUD_WLAN_DNS_DEFAULT_NUMBER);
	if(dns_white_list.addr == NULL)
	{
		return CWLAN_FAIL;
	}
	memset(&dns_white_list, 0, sizeof(dns_white_list));

	//获取本地ip地址，要不访问不了管理页面
	get_interface_info("br-lan", &ip, NULL);
	dns_white_list.addr[dns_white_list.number++] = ip;

	/*更新portal的url*/
	cw_ap_local_db_portal_list_update();
	//更新数据库的url白名单
	cw_ap_local_db_url_wl_update(&dns_white_list);
	
	cloud_wlan_sendto_kmod(CW_NLMSG_UPDATE_URL_WHITE_LIST, (s8 *)&dns_white_list, sizeof(dns_white_list_t)+CLOUD_WLAN_DNS_DEFAULT_NUMBER*sizeof(u32));

	free(dns_white_list.addr);
    return CWLAN_OK;
}

u32 cw_ap_local_db_user_white_list_update(sqlite3 * db)
{
	sqlite3_res res;
	user_wl_buf_t ubuff={1, NULL};
	u32 ret;
	u32 i;
	//在用户白名单里并不需要时间,只有认证用户需要
	snprintf(g_cw_sql, G_CW_SQL_BUF_LEN, "select mac from %s;", CWLAN_AP_USER_WHITE_TABLE);
	ret = sqlite3_exec_get_res(g_cw_db, g_cw_sql, &res);
	if(ret != CWLAN_OK)
	{
		cw_local_log_writeto_file("sqlite3_exec_get_res[%s]: %s\n",g_cw_sql, sqlite3_errmsg(g_cw_db));  
		return CWLAN_FAIL;
	}
	for(i=1; i<=res.nrow; i++)
	{
		sqlite3_get_s8(&res, i, "mac", &ubuff->buf);
	}
	cloud_wlan_sendto_kmod(CW_NLMSG_USER_WHITE_LIST_ADD, (s8 *)ubuff->buf, strlen(buf)+sizeof(user_wl_buf_t));

	sqlite3_exec_free_res(&res);
}
u32 cw_ap_local_db_get_online_init(sqlite3 * db)
{
	sqlite3_res res;
	s8 *buf=NULL;
	u32 ret;
	
  	snprintf(g_cw_sql, G_CW_SQL_BUF_LEN, "select * from %s;", CWLAN_AP_ONLINE_DB_TABLE);
	ret = sqlite3_exec_get_res(db, g_cw_sql, &res);
	if(ret != CWLAN_OK)
	{
		printf("INIT:sqlite3_exec_get_res[%s]: %s\n",g_cw_sql, sqlite3_errmsg(g_cw_db));  
		exit(1);
	}

	
	sqlite3_get_u8(&res, 1, "server_addr", (u8 **)&buf);
	memcpy(g_online_socket.server_addr,buf, 20);
	sqlite3_get_u32(&res, 1, "server_port", &g_online_socket.server_port);

	printf("INIT:cwlan server_addr          %s\n", g_online_socket.server_addr);
	printf("INIT:cwlan server_port          %d\n", g_online_socket.server_port);

	sqlite3_exec_free_res(&res);
	return CWLAN_OK;
}
u32 cw_ap_local_get_db_log_init(sqlite3 * db)
{
	sqlite3_res res;
	s8 *buf=NULL;
	u32 ret;
	
	u8 server_addr[32];
	u32 server_port;

  	snprintf(g_cw_sql, G_CW_SQL_BUF_LEN, "select * from %s;", CWLAN_AP_LOG);
	ret = sqlite3_exec_get_res(db, g_cw_sql, &res);
	if(ret != CWLAN_OK)
	{
		printf("INIT:sqlite3_exec_get_res[%s]: %s\n",g_cw_sql, sqlite3_errmsg(g_cw_db));  
		exit(1);
	}

	
	sqlite3_get_u32(&res, 1, "ulog_sw", &g_log.ulog_sw);
	sqlite3_get_u32(&res, 1, "klog_sw", &g_log.klog_sw);
	sqlite3_get_u8(&res, 1, "server_addr", (u8 **)&buf);
	memcpy(g_log.server_addr,buf, 20);
	sqlite3_get_u32(&res, 1, "server_port", &g_log.server_port);
	sqlite3_get_u8(&res, 1, "server_path", (u8 **)&buf);
	memcpy(g_log.server_path,buf, 128);
	sqlite3_get_u8(&res, 1, "server_user", (u8 **)&buf);
	memcpy(g_log.server_user,buf, 32);
	sqlite3_get_u8(&res, 1, "server_password", (u8 **)&buf);
	memcpy(g_log.server_password,buf, 64);

	printf("INIT:cwlan ulog_sw             %s\n", g_log.ulog_sw);
	printf("INIT:cwlan klog_sw             %s\n", g_log.klog_sw);
	printf("INIT:cwlan server_addr         %s\n", g_log.server_addr);
	printf("INIT:cwlan server_port         %d\n", g_log.server_port);
	printf("INIT:cwlan server_path         %s\n", g_log.server_path);
	printf("INIT:cwlan server_user         %s\n", g_log.server_user);
	printf("INIT:cwlan server_password     %s\n", g_log.server_password);

	sqlite3_exec_free_res(&res);
	return CWLAN_OK;
}

u32 cw_ap_local_db_cfg_init(sqlite3 * db)
{
	cwlan_base_cfg_t temp_session_cfg;

	//先关闭
	cloud_wlan_sendto_kmod(CW_NLMSG_SET_OFF, NULL, 0);
	
	cw_ap_local_db_dns_update(db);
	cw_ap_local_db_user_white_list_update(db);
	cw_ap_local_db_get_online_init(db);


	
	cw_ap_local_get_db_config_base(db);
	cw_ap_local_get_db_log_init(db);
	temp_session_cfg.usage_model = g_config_base.usage_model;
	temp_session_cfg.cwlan_sw = g_config_base.cwlan_sw;
	temp_session_cfg.klog_sw = g_log.klog_sw;
	temp_session_cfg.over_time = g_config_base.time_online;
	temp_session_cfg.interval_timer = g_config_base.interval_timer;
	
	cloud_wlan_sendto_kmod(CW_NLMSG_UPDATE_SESSION_CFG, (s8 *)&temp_session_cfg, sizeof(temp_session_cfg));

	return CWLAN_OK;
}


void *cw_ap_local_cfg_update_pthread(void *param)
{
	u32 i, ret;
	time_t cw_network = 0;
	s8 *url = NULL;
	u32 ip = 0;
	s8 cmd[128]={0};
	s8 logname[64]={0};
	struct in_addr tmp_ip;
	//sqlite3_res res;
	dns_white_list_t dns_white_list = {0,NULL};
	struct stat filestat;

	while(1)
	{
	
/*这个需要系统功能支持*/
		stat(DNS_CONFIG_FILE, &filestat);
		if(cw_network < filestat.st_ctime)
		{
			cw_network = filestat.st_ctime;
			memset(&dns_white_list, 0, sizeof(dns_white_list));
/*优先更新本地，要不管理web页面都上不去*/
			get_interface_info("br-lan", &ip, NULL);
			dns_white_list.addr[dns_white_list.number++] = ip;
			cloud_wlan_sendto_kmod(CW_NLMSG_UPDATE_URL_WHITE_LIST, (s8 *)&dns_white_list, sizeof(dns_white_list_t));
			
			while(1)
			{
				get_interface_info(g_config_base.wan_eth_name, &ip, NULL);
				if(ip != 0)
				{
					break;
				}
			}

			/*更新portal的url*/
			cw_ap_local_db_dns_update(g_cw_db);

		}

		sleep(3);
	};
}

static u32 cw_ap_sendto_ac_local_info(dcma_udp_skb_info_t *buff)
{
	return CWLAN_OK;
}
extern u32 (*cw_ap_sendto_ac_info)(dcma_udp_skb_info_t *buff);

u32 cw_local_dispose_pthread_cfg_init(struct pthread_id *g_pthread)
{
	u32 ret = 0;
		
	pthread_attr_t attr;
	//struct sched_param  params;
	
	ret = pthread_attr_init(&attr);
	
	//线程退出直接释放资源
	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	/* 本地接收内核数据线程*/
	cw_ap_sendto_ac_info = cw_ap_sendto_ac_local_info;
	ret = pthread_create(&g_pthread->recv_k_id,&attr,cloud_wlan_recv_kmod_info,NULL);
	if(ret!=0){
		printf("init cloud_wlan_recv_kmod_info fail %d!\n",ret);
	}
	ret = pthread_create(&g_pthread->update_id,NULL,cw_ap_local_cfg_update_pthread,NULL);
	if(ret!=0){
		printf("init cw_ap_local_cfg_update_pthread fail %d!\n",ret);
	}
	return ret;
}

u32 cw_local_dynamic_update_init(struct pthread_id *g_pthread)
{

	return cw_local_dispose_pthread_cfg_init(g_pthread); 
}
u32 cw_local_dynamic_update_exit()
{


}

