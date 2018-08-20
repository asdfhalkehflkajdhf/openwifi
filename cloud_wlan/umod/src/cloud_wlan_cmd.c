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
#include <sys/stat.h>
#include <stdint.h>
#include <netdb.h>

#include "cloud_wlan_types.h"
#include "cloud_wlan_nl_out_pub.h"
#include "cloud_wlan_sqlite.h"
#include "cloud_wlan_ap_com.h"
#include "cloud_wlan_ap_local.h"




sqlite3 *db;


u32 cw_debug_config_base_help()
{
	printf("config_base\n");
		printf(" usage_model <local|online>\n");
		printf(" cwlan_sw <on|off>\n");
		printf(" time_online <time s>\n");
		printf(" interval_timer <time s>\n");
		printf(" wan_eth_name <data>\n");
		printf(" show\n");
	return CWLAN_OK;
}

u32 cw_debug_user_white_list_help()
{
	printf("user_white_list\n");
		printf(" add <mac> <portal_id> <time_online>\n");
		printf(" del <mac>\n");
		printf(" show\n");
	return CWLAN_OK;
}
u32 cw_debug_url_white_list_help()
{
	printf("url_white_list\n");
		printf(" add <url>\n");
		printf(" del <id>\n");
		printf(" show\n");
		return CWLAN_OK;
}
u32 cw_debug_portal_list_help()
{
	printf("portal_list\n");
		printf(" add <url>\n");
		printf(" del <id>\n");
		printf(" show\n");
	return CWLAN_OK;
}
u32 cw_debug_show_kmod_help()
{
	printf("show_kmod");
		printf("	<url_p> 			ap show protal url\n");
		printf("	<url_wl>			ap show url white list\n");
		printf("	<user_wl>			ap show user white list\n");
		printf("	<online>			ap show online user info\n");
	return CWLAN_OK;
}

u32 cw_debug_log_help()
{
	printf("log\n");
		printf(" ulog_sw <on|off>\n");
		printf(" klog_sw <on|off>\n");
		printf(" log_mode <real|unreal>\n");
		printf(" server_addr <data>\n");
		printf(" server_port <data>\n");
		printf(" server_path <data>\n");
		printf(" server_user <data>\n");
		printf(" server_password <data>\n");
		printf(" show\n");
	return CWLAN_OK;
}

u32 cw_debug_online_help()
{
	printf("online\n");
		printf(" server_addr <data>\n");
		printf(" server_port <data>\n");
		printf(" show\n");
	return CWLAN_OK;
}

u32 cw_debug_show_help()
{
	
	cw_debug_config_base_help();
	cw_debug_user_white_list_help();
	cw_debug_url_white_list_help();
	cw_debug_portal_list_help();
	cw_debug_log_help();
	cw_debug_show_kmod_help();
	
	printf("commit\n");
	printf("test\n");
	
	return CWLAN_OK;
}

u32 cw_set_config_base(char **argv)
{
	if(!strcmp(argv[2], "usage_model"))
	{
		if(!strcmp(argv[3], "local"))
			sqlite3_update_date(db,CWLAN_AP_CFG_TABLE,"%s = %d", argv[2], USAGE_MODEL_LOCAL);
		else if(!strcmp(argv[3], "online"))
			sqlite3_update_date(db,CWLAN_AP_CFG_TABLE,"%s = %d", argv[2], USAGE_MODEL_ONLINE);
		goto flag;
	}
	if(!strcmp(argv[2], "cwlan_sw"))
	{
		if(!strcmp(argv[3], "on"))
			sqlite3_update_date(db,CWLAN_AP_CFG_TABLE,"%s = %d", argv[2], CWLAN_OPEN);
		else if(!strcmp(argv[3], "off"))
			sqlite3_update_date(db,CWLAN_AP_CFG_TABLE,"%s = %d", argv[2], CWLAN_CLOSE);
		goto flag;
	}
	if(!strcmp(argv[2], "time_online"))
	{
		sqlite3_update_date(db,CWLAN_AP_CFG_TABLE,"%s = %s", argv[2], argv[3]);
		goto flag;
	}
	if(!strcmp(argv[2], "interval_timer"))
	{
		sqlite3_update_date(db,CWLAN_AP_CFG_TABLE,"%s = %s", argv[2], argv[3]);
		goto flag;
	}
	if(!strcmp(argv[2], "wan_eth_name"))
	{
		sqlite3_update_date(db,CWLAN_AP_CFG_TABLE,"%s = %s", argv[2], argv[3]);
		goto flag;
	}
	if(!strcmp(argv[2], "show"))
	{
		sqlite3_table_show(db,CWLAN_AP_CFG_TABLE,"*", "");
		goto flag;
	}

	cw_debug_config_base_help();
flag:
	return CWLAN_OK;
}

u32 cw_set_online(char **argv)
{
	if(!strcmp(argv[2], "server_addr"))
	{
		sqlite3_update_date(db,CWLAN_AP_ONLINE_DB_TABLE,"%s = %s", argv[2], argv[3]);
		goto flag;
	}
	if(!strcmp(argv[2], "server_port"))
	{
		sqlite3_update_date(db,CWLAN_AP_ONLINE_DB_TABLE,"%s = %s", argv[2], argv[3]);
		goto flag;
	}
	if(!strcmp(argv[2], "show"))
	{
		sqlite3_table_show(db,CWLAN_AP_ONLINE_DB_TABLE,"*", "");
		goto flag;
	}
	cw_debug_online_help();
flag:
	return CWLAN_OK;
}

u32 cw_set_user_white_list(char **argv)
{
	if(!strcmp(argv[2], "add"))
	{
		sqlite3_insert_date(db,CWLAN_AP_USER_WHITE_TABLE,"mac, portal_id, time_online", "'%s' ,%s , %s", argv[3], argv[4], argv[5]);
		goto flag;
	}
	if(!strcmp(argv[2], "del"))
	{
		sqlite3_delete_date(db,CWLAN_AP_USER_WHITE_TABLE,"id = %s", argv[3]);
		goto flag;
	}
	if(!strcmp(argv[2], "show"))
	{
		sqlite3_table_show(db,CWLAN_AP_USER_WHITE_TABLE,"*", "");
		goto flag;
	}
	cw_debug_user_white_list_help();
flag:
	return CWLAN_OK;
}

u32 cw_set_url_white_list(char **argv)
{
	if(!strcmp(argv[2], "add"))
	{
		sqlite3_insert_date(db,CWLAN_AP_URL_WHITE_TABLE,"url","%s",  argv[3]);
		goto flag;
	}
	if(!strcmp(argv[2], "del"))
	{
		sqlite3_delete_date(db,CWLAN_AP_URL_WHITE_TABLE,"id = %s", argv[3]);
		goto flag;
	}
	if(!strcmp(argv[2], "show"))
	{
		sqlite3_table_show(db,CWLAN_AP_URL_WHITE_TABLE,"*", "");
		goto flag;
	}
	cw_debug_url_white_list_help();
flag:
	return CWLAN_OK;
}
u32 cw_set_portal_list(char **argv)
{
	if(!strcmp(argv[2], "add"))
	{
		sqlite3_insert_date(db,CWLAN_AP_PORTAL_TABLE,"url","%s",  argv[3]);
		goto flag;
	}
	if(!strcmp(argv[2], "del"))
	{
		sqlite3_delete_date(db,CWLAN_AP_PORTAL_TABLE,"id = %s", argv[3]);
		goto flag;
	}
	if(!strcmp(argv[2], "show"))
	{
		sqlite3_table_show(db,CWLAN_AP_PORTAL_TABLE,"*", "");
		goto flag;
	}
	cw_debug_portal_list_help();
flag:
	return CWLAN_OK;
}

u32 cw_set_log(char **argv)
{

	if(!strcmp(argv[2], "ulog_sw"))
	{
		if(!strcmp(argv[3], "on"))
			sqlite3_update_date(db,CWLAN_AP_LOG,"%s = %d", argv[2], CWLAN_OPEN);
		else if(!strcmp(argv[3], "off"))
			sqlite3_update_date(db,CWLAN_AP_LOG,"%s = %d", argv[2], CWLAN_CLOSE);
		goto flag;
	}
	if(!strcmp(argv[2], "klog_sw"))
	{
		if(!strcmp(argv[3], "on"))
			sqlite3_update_date(db,CWLAN_AP_LOG,"%s = %d", argv[2], CWLAN_OPEN);
		else if(!strcmp(argv[3], "off"))
			sqlite3_update_date(db,CWLAN_AP_LOG,"%s = %d", argv[2], CWLAN_CLOSE);
		goto flag;
	}
	if(!strcmp(argv[2], "log_mode"))
	{
		if(!strcmp(argv[3], "real"))
			sqlite3_update_date(db,CWLAN_AP_LOG,"%s = %d", argv[2], REAL_TIME);
		else if(!strcmp(argv[3], "unreal"))
			sqlite3_update_date(db,CWLAN_AP_LOG,"%s = %d", argv[2], UNREAL_TIME);
		goto flag;
	}
	if(!strcmp(argv[2], "server_addr"))
	{
		sqlite3_update_date(db,CWLAN_AP_LOG,"%s = %s", argv[2], argv[3]);
		goto flag;
	}
	if(!strcmp(argv[2], "server_port"))
	{
		sqlite3_update_date(db,CWLAN_AP_LOG,"%s = %s", argv[2], argv[3]);
		goto flag;
	}
	if(!strcmp(argv[2], "server_path"))
	{
		sqlite3_update_date(db,CWLAN_AP_LOG,"%s = %s", argv[2], argv[3]);
		goto flag;
	}
	if(!strcmp(argv[2], "server_user"))
	{
		sqlite3_update_date(db,CWLAN_AP_LOG,"%s = %s", argv[2], argv[3]);
		goto flag;
	}
	if(!strcmp(argv[2], "server_password"))
	{
		sqlite3_update_date(db,CWLAN_AP_LOG,"%s = %s", argv[2], argv[3]);
		goto flag;
	}
	if(!strcmp(argv[2], "show"))
	{
		sqlite3_table_show(db,CWLAN_AP_LOG,"*", "");
		goto flag;
	}

	cw_debug_log_help();
flag:
	return CWLAN_OK;
}
u32 cw_set_show_kmod(s8 *cmd)
{
	if(!strcmp(cmd, "url_p"))
	{
		cloud_wlan_sendto_kmod(CW_NLMSG_DEBUG_SHOW_PORTAL, NULL, 0);
		goto flag;
	}
	if(!strcmp(cmd, "url_wl"))
	{
		cloud_wlan_sendto_kmod(CW_NLMSG_DEBUG_SHOW_URL_WHITE_LIST, NULL, 0);
		goto flag;
	}
	if(!strcmp(cmd, "online"))
	{
		cloud_wlan_sendto_kmod(CW_NLMSG_DEBUG_SHOW_ONLINE_USER, NULL, 0);
		goto flag;
	}
	if(!strcmp(cmd, "user_wl"))
	{
		cloud_wlan_sendto_kmod(CW_NLMSG_DEBUG_SHOW_USER_WHITE_LIST, NULL, 0);
		goto flag;
	}
	cw_debug_show_kmod_help();
flag:
	return CWLAN_OK;
}
#if 0
u32 cw_debug_des_process(s8 *cmd, s8 *data)
{
	s8 out[CW_DES_LEN]={0};
	s8 *decryt;
	u32 ret;
	sqlite3 *db;
	u32 data_len;

	ret =sqlite3_open(CWLAN_AP_CFG_DB, &db);
	if( ret )						//如果出错，给出提示信息并退出程序	
	{
		printf("INIT:Can'topen database: %s\n", sqlite3_errmsg(db));  
		sqlite3_close(db); 
		return CWLAN_OK;
	}

	if(!strcmp(cmd, "def"))
	{
		DES_Act(out, CW_LOCATION_URL_DATA, strlen(CW_LOCATION_URL_DATA),g_des_key, DES_KEY_LEN, ENCRYPT);
/*目前不区分portal类型*/
		snprintf(g_cw_sql, G_CW_SQL_BUF_LEN, 
			"UPDATE %s SET url=?;",CWLAN_AP_PORTAL_TABLE);
		sqlite3_binary_write1(db,g_cw_sql,out, CW_DES_LEN);
		sqlite3_close(db);
		return CWLAN_OK;
	}
	if(!strcmp(cmd, "e") && data != NULL)
	{
		DES_Act(out, data, strlen(data),g_des_key, DES_KEY_LEN, ENCRYPT);

		snprintf(g_cw_sql, G_CW_SQL_BUF_LEN, "UPDATE %s SET url=?;",CWLAN_AP_PORTAL_TABLE);
		sqlite3_binary_write1(db,g_cw_sql,out, CW_DES_LEN);
		sqlite3_close(db); 
		return CWLAN_OK;
	}
	if(!strcmp(cmd, "d"))
	{
		snprintf(g_cw_sql, G_CW_SQL_BUF_LEN, "select url from %s;", CWLAN_AP_PORTAL_TABLE);

		data_len = sqlite3_binary_read(db, g_cw_sql, &decryt, 0);

		DES_Act(out, decryt, data_len,g_des_key, DES_KEY_LEN, DECRYPT);
		printf("%d %s\n",data_len, out);

		sqlite3_close(db); 
		return CWLAN_OK;
	}
	cw_debug_show_help();
	
	sqlite3_close(db); 
	return CWLAN_OK;
}
#endif
u32 cw_debug_test_process(s8 *data)
{
	cloud_wlan_sendto_kmod_ok(CW_NLMSG_GET_TEST, data, strlen(data));
	
	return CWLAN_OK;
}
u32 cw_debug_branch(char **argv)
{
	if(!strcmp(argv[1], "config_base"))
	{
		cw_set_config_base(argv);
		goto quit;
	}
	if(!strcmp(argv[1], "portal_list"))
	{
		cw_set_portal_list(argv);
		goto quit;
	}
	
	if(!strcmp(argv[1], "online"))
	{
		cw_set_online(argv);
		goto quit;
	}
	
	if(!strcmp(argv[1], "user_white_list"))
	{
		cw_set_user_white_list(argv);
		goto quit;
	}
	if(!strcmp(argv[1], "url_white_list"))
	{
		cw_set_url_white_list(argv);
		goto quit;
	}
	if(!strcmp(argv[1], "log"))
	{
		cw_set_log(argv);
		goto quit;
	}
	

	if(!strcmp(argv[1], "commit"))
	{
		cw_ap_local_db_cfg_init(db);
		goto quit;
	}
	if(!strcmp(argv[1], "test"))
	{
		cw_debug_test_process(argv[2]);
		goto quit;
	}
	
	if(!strcmp(argv[1], "show_kmod"))
	{
		cw_set_show_kmod(argv[2]);
		goto quit;
	}
	if(!strcmp(argv[1], "help"))
	{
		cw_debug_show_help();
		goto quit;
	}
	
quit:
	return CWLAN_OK;

}
int main(int argc, char **argv)  
{
	int ret;
	if( argc<2 )						//如果出错，给出提示信息并退出程序	
	{
		cw_debug_show_help();
		return 0;
	}
	cloud_wlan_nl_cfg_init();

	ret =sqlite3_open(CWLAN_AP_CFG_DB, &db);
	if( ret )						//如果出错，给出提示信息并退出程序	
	{
		printf("INIT:Can'topen database: %s\n", sqlite3_errmsg(db));  
		sqlite3_close(db); 
		return CWLAN_OK;
	}

	cw_debug_branch(argv);
	sqlite3_close(db); 
	cloud_wlan_nl_close();
	return CWLAN_OK;
}

