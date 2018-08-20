#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>


#include <stdarg.h>
#include <mysql/mysql.h>
#include "collect.h"
int g_cw_debug = 1;

char collect_service[100]={0};
char collect_table_name[100]={0};
char collect_act_dir[100]={0};
char collect_table_dir[100]={0};
char user_name[50]={0};
char password[50]={0};
char mysql_db[50]={0};
int interval = 0;



MYSQL *conn; 
MYSQL_RES *res; 
MYSQL_ROW row; 

char sql[COLLECT_MYSQL_LEN];


int get_one_recode_info(FILE *fd, record_t *node)
{
	char buff[256]={0};   
	char tmp1[50];
	unsigned int kflag = 0;
	int enterdb = 0;

	memset((void *)node, 0, sizeof(record_t));
	
	while(!feof(fd) && buff[0]!= '\r' && buff[0] != '\n')
	{
		fgets (buff, sizeof(buff), fd); 
		//DEBUG_U("%d ",get_bit(kflag)+1);
		switch(get_bit(kflag)+1)
		{
			case 1:
				if(buff[0] > '0' && buff[0] < '9')
				{
					kflag = set_bit(kflag,get_bit(kflag));
					snprintf(node->action_request_time, 20, buff);
					break;
				}
			case 2:
				if( !memcmp(buff, "\tFramed-IP-Address", 18))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %s",tmp1,node->Framed_IP_Address); 
					break;
				}
			case 3:
				if( !memcmp(buff, "\tAcct-Session-Id", 16))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %s",tmp1,node->Acct_Session_Id); 
					break;
				}
			case 4:
				if( !memcmp(buff, "\tNAS-Port", 9))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %d",tmp1,&node->NAS_Port); 
					break;
				}
			case 5:
				if( !memcmp(buff, "\tNAS-Port-Type", 14))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %s",tmp1,node->NAS_Port_Type); 
					break;
				}
			case 6:
				if( !memcmp(buff, "\tCalling-Station-Id", 19))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %s",tmp1,node->Calling_Station_Id); 
					break;
				}
			case 7:
				
				if( !memcmp(buff, "\tAcct-Delay-Time", 16))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %d",tmp1,&node->Acct_Delay_Time); 
					break;
				}
			case 8:
				if( !memcmp(buff, "\tAcct-Status-Type", 17))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %s",tmp1,node->acct_status_type);
					if(0 == memcmp(node->acct_status_type, "Stop", 4) 
						 || 0 == memcmp(node->acct_status_type, "Start", 5))
					{
						DEBUG_U("%s\n",node->acct_status_type);
						enterdb = 1;
						break;
					}
					//过滤radius其他属性
					while(!feof(fd) && buff[0]!= '\r' && buff[0] != '\n')
					{
						fgets(buff, sizeof(buff), fd);
					}
					break;
				}
			case 9:
				if( !memcmp(buff, "\tAcct-Session-Time", 18))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %d",tmp1,&node->Acct_Session_Time); 
					break;
				}
			case 10:
				if( !memcmp(buff, "\tAcct-Input-Octets", 18))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %d",tmp1,&node->Acct_Input_Octets); 
					break;
				}
			case 11:
				if( !memcmp(buff, "\tAcct-Input-Gigawords", 21))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %d",tmp1,&node->Acct_Input_Gigawords); 
					break;
				}
			case 12:
				if( !memcmp(buff, "\tAcct-Output-Octets", 19))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %d",tmp1,&node->Acct_Output_Octets); 
					break;
				}
			case 13:
				if( !memcmp(buff, "\tAcct-Output-Gigawords", 22))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %d",tmp1,&node->Acct_Output_Gigawords); 
					break;
				}
			case 14:
				if( !memcmp(buff, "\tAcct-Input-Packets", 19))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %d",tmp1,&node->Acct_Input_Packets); 
					break;
				}
			case 15:
				if( !memcmp(buff, "\tAcct-Output-Packets", 20))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %d",tmp1,&node->Acct_Output_Packets); 
					break;
				}
			case 16:
				if( !memcmp(buff, "\tProxy-State", 12))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %d",tmp1,&node->Proxy_State); 
					break;
				}
			case 17:
				if( !memcmp(buff, "\tUser-Name", 10))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = \"%s\"",tmp1,node->User_Name); 
					break;
				}		
			case 18:
				if( !memcmp(buff, "\tNAS-IP-Address", 15))
				{
					kflag = set_bit(kflag,get_bit(kflag));
					sscanf(buff, "%s = %s",tmp1,node->NAS_IP_Address); 
					break;
				}
					
			default:
				break;
		}

	}

	return enterdb;
}

int read_collect_and_enter_db(collect_file_info_t * collect_name)
{
	FILE *fd;
	record_t node;
	int flen;
	int ret;
	char path[50]={0};
	char *string_res[3]={NULL};

	snprintf(path, 50, "%s%04d-%02d-%02d.act",collect_act_dir ,collect_name->yesr, collect_name->month, collect_name->day);


	fd = fopen (path, "r"); 
	if(fd == NULL)
	{
		DEBUG_U("open fail %s\n",path);
		return 0;
	}
	fseek(fd, 0, SEEK_END);
	flen=ftell(fd);
	if(flen <= collect_name->offset)
	{
		fclose(fd);
		DEBUG_U("current filelen %d <= offset %d\n",flen, collect_name->offset);
		return 0;
	}
	//移动到文件上次读的指针地址
	fseek(fd, 0, SEEK_SET);
	fseek(fd, collect_name->offset, SEEK_SET);
	
	while(!feof(fd))
	{
		ret = get_one_recode_info(fd, &node);
		if(ret != 0)
		{
			ret = split_string('-', node.User_Name, 3, string_res);
			//if(ret != 3)
			if(0)
			{
				DEBUG_U("ret = %d\n", ret);
				break;
			}
			snprintf(sql, 1024, 
				"insert into %s%04d%02d ("
				"framed_ip_address, acct_session_id, nas_port,nas_port_type, calling_station_id,"
				"acct_delay_time,action_request_time, acct_status_type, acct_session_time,"
				"acct_input_octets,	acct_input_gigawords, acct_output_octets, acct_output_gigawords,"
				"acct_input_packets, acct_output_packets, proxy_state, user_name,"
				"nas_ip_address, user_type, company_id )VALUES("
				"'%s', %s, %d, '%s', %s, "
				"%d, '%s', '%s', %d,"
				"%d, %d, %d, %d, "
				"%d, %d, %d, '%s',"
				"'%s', %s, %s );",
				collect_table_name, collect_name->yesr, collect_name->month,
				node.Framed_IP_Address, node.Acct_Session_Id, node.NAS_Port,node.NAS_Port_Type, node.Calling_Station_Id,
				node.Acct_Delay_Time, node.action_request_time, node.acct_status_type, node.Acct_Session_Time,
				node.Acct_Input_Octets, node.Acct_Input_Gigawords, node.Acct_Output_Octets, node.Acct_Output_Gigawords,
				node.Acct_Input_Packets, node.Acct_Output_Packets, node.Proxy_State, node.User_Name,
				node.NAS_IP_Address, string_res[1], string_res[2]);

			/* send SQL query */ 
			if (mysql_query(conn, sql)) { 
				DEBUG_U("%s\n",sql);
				DEBUG_U("%s\n", mysql_error(conn)); 
			} 
			DEBUG_U("\n");
		}
	}
	//获得本次最后读到的文件指针地址
	collect_name->offset = ftell(fd);
	fclose(fd);
	return 0;
}
int collect_file_info_init(collect_file_info_t *collect_info)
{
	struct tm *newtime;
	newtime = localtime(&collect_info->sec);
	collect_info->yesr = newtime->tm_year +1900;
	collect_info->month= newtime->tm_mon + 1;
	collect_info->day= newtime->tm_mday;
	return 0;
}
int update_before_collect(collect_file_info_t *before_name)
{
	time_t now;
	time(&now);
	int tot_bday = before_name->sec/(60*60*24);
	int tot_cday = now/(60*60*24);
	//因为get_offset结果为不为0上次有数据
	while(tot_bday != 0 && tot_bday < tot_cday)
	{
		collect_file_info_init(before_name);
		collect_create_table(before_name);
		read_collect_and_enter_db(before_name);
		
		DEBUG_U("%s update_before_collect_data %d %d %d \n",
			ctime(&now), before_name->yesr, before_name->month, before_name->day);
		tot_bday++;
		before_name->sec = before_name->sec + (time_t)(60*60*24);
		before_name->offset = 0;
	}
	return 0;
}
int update_current_collect(collect_file_info_t *current_name)
{
	static collect_file_info_t before_info = {0,0,0,0,0};
	
	//换天需要重围offset,如果before.day==0 则没有上一天的数据
	if(before_info.day != 0 && before_info.day != current_name->day)
	{
		//把上一天的数据读完
		DEBUG_U("Update the data before today %04d%02d%02d\n", current_name->yesr,current_name->month,current_name->day);
		read_collect_and_enter_db(&before_info);
		current_name->offset = 0;
	}
	//换月需要更新表
	if(before_info.month != current_name->month)
	{
		DEBUG_U("collect_create_table %04d%02d\n", current_name->yesr, current_name->month);
		collect_create_table(current_name);
	}
	read_collect_and_enter_db(current_name);
		
	memcpy(&before_info, current_name, sizeof(collect_file_info_t));
	//保存上次读的文件和读到的地方
	set_offset(current_name);
	return 0;
}

int config_init(char *path)
{
	FILE *fd;

	char buff[64];
	char tmp1[64];
	
	fd = fopen (path, "r"); 
	if(fd == NULL)
	{
		printf("open fail %s\n",path);
		return 1;
	}

	while(!feof(fd))
	{
		fgets (buff, sizeof(buff), fd); 

		if( !memcmp(buff, "collect_act_dir", strlen("collect_act_dir")))
		{
			sscanf(buff, "%s = %s",tmp1,collect_act_dir); 
			continue;
		}
		if( !memcmp(buff, "collect_table_dir", strlen("collect_table_dir")))
		{
			sscanf(buff, "%s = %s",tmp1,collect_table_dir); 
			continue;
		}
		if( !memcmp(buff, "collect_service", strlen("collect_service")))
		{
			sscanf(buff, "%s = %s",tmp1,collect_service); 
			continue;
		}
		
		if( !memcmp(buff, "collect_table_name", strlen("collect_table_name")))
		{
			sscanf(buff, "%s = %s",tmp1,collect_table_name); 
			continue;
		}
		if( !memcmp(buff, "user_name", strlen("user_name")))
		{
			sscanf(buff, "%s = %s",tmp1,user_name); 
			continue;
		}
		if( !memcmp(buff, "password", strlen("password")))
		{
			sscanf(buff, "%s = %s",tmp1,password); 
			continue;
		}
		if( !memcmp(buff, "mysql_db", strlen("mysql_db")))
		{
			sscanf(buff, "%s = %s",tmp1,mysql_db); 
			continue;
		}
		if( !memcmp(buff, "interval", strlen("interval")))
		{
			sscanf(buff, "%s = %d",tmp1,&interval); 
			continue;
		}
		if( !memcmp(buff, "debug_log", strlen("debug_log")))
		{
			sscanf(buff, "%s = %d",tmp1,&g_cw_debug); 
			continue;
		}
		
	}
	fclose(fd);
	if(collect_act_dir[0] == '\0' || collect_table_name[0] == '\0' || collect_service[0] == '\0' 
	|| user_name[0] == '\0' || mysql_db[0] == '\0' || collect_table_dir[0]=='\0')
	{
		printf("config file is error\n");
		return 1;
	}
	printf("collect_act_dir    : %s\n", collect_act_dir);
	printf("collect_table_name : %s\n", collect_table_name);
	printf("collect_table_dir  : %s\n", collect_table_dir);
	printf("user_name          : %s\n", user_name);
	printf("password           : %s\n", password);
	printf("mysql_db           : %s\n", mysql_db);
	printf("interval           : %d\n", interval);
	return 0;
}

int main(int argc,char *argv[])
{
	int ret;
	collect_file_info_t before_name;
	collect_file_info_t current_name;
/*不设置成守护进程可能会被系统kill*/
	if(daemon(1, 1) != 0)
    {
    	printf("daemon fail\n");
        return 0;
    }

	if(argc < 2)
	{
		printf("%s <config file path> \n", argv[0]);
		return 0;
	}

	ret = config_init(argv[1]);
	if(ret != 0)
	{
		printf("config_init fail\n");
		return 0;
	}

    conn = mysql_init(NULL); 
	/* Connect to database */ 
	if (!mysql_real_connect(conn, collect_service, 
		   user_name, password, mysql_db, 0, NULL, 0)) { 
	   fprintf(stderr, "%s\n", mysql_error(conn)); 
	   return 0; 
	}

/*这个程序存在长时间不执行sql时会自动断开问题*/
	//读取上次读的文件和读到的地方
	get_offset(&before_name);
	current_name.offset=before_name.offset;
	//更新上次未读完数据到当前日期
	update_before_collect(&before_name);
	DEBUG_U("collect start!\n\n");

	while(1)
	{
		//更新当前数据
		time(&current_name.sec);
		collect_file_info_init(&current_name);
		update_current_collect(&current_name);
		sleep(interval);
	}
	return 0;
}
