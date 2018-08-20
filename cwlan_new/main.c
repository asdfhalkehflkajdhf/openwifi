#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
//#include <sys/socket.h>
//#include <linux/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <mysql/mysql.h>
#include <error.h>
#include "create_dir.h"

int g_connection_state_db = 0;
char update_k[100000]={0};

int listenport = 22222;


#define mysql_communication_table "T_communicat"
#define mysql_admin_table "T_admin"
#define mysql_ap_base_config_table "T_ap_base_config"
#define mysql_ap_group_list_table "T_ap_group_list"
#define mysql_ap_info_table "T_ap_info"
#define mysql_ap_portal_list_table "T_ap_portal_list"
#define mysql_url_white_list_table "T_ap_url_white_list"
#define mysql_communication_table "T_communicat"
#define mysql_user_online_log_table "T_user_online_log"
#define mysql_user_url_log_table "T_user_url_log"
#define mysql_user_white_list_table "T_user_white_list"

void data_db_to_ap_branch()
{

}


char db_service[100]="localhost";
char user_name[50]="root";
char password[50]="lzc123";
char mysql_db[50]="cwlan";
int db_port = 3306;

void * data_db_to_ap(void *arg)
{
	int sockfd = -1;
	int recv_len=0;
	struct sockaddr_in server_addr, client_addr;
	int sin_size = sizeof(struct sockaddr);
	memcpy((void *)&client_addr, arg, sizeof(struct sockaddr_in));

	
	MYSQL *conn; 
	MYSQL_RES *res; 
	MYSQL_ROW row; 
	char sql[128];
    conn = mysql_init(NULL); 
	/* Connect to database */ 
	if (!mysql_real_connect(conn, db_service, 
		   user_name, password, mysql_db, db_port, NULL, 0)) { 
	   fprintf(stderr, "%s\n", mysql_error(conn)); 
	   return 0; 
	}

	data_db_to_ap_branch();
	mysql_close(conn);

	return NULL;
}


void data_ap_to_db_branch()
{

}
void * data_ap_to_db(void *arg)
{
	int sockfd = -1;
	int recv_len=0;
	struct sockaddr_in server_addr, client_addr;
	int sin_size = sizeof(struct sockaddr);
	

	MYSQL *conn; 
	MYSQL_RES *res; 
	MYSQL_ROW row; 
	char sql[128];
    conn = mysql_init(NULL); 
	/* Connect to database */ 
	if (!mysql_real_connect(conn, db_service, 
		   user_name, password, mysql_db, db_port, NULL, 0)) { 
	   fprintf(stderr, "%s\n", mysql_error(conn)); 
	   return 0; 
	}

	data_ap_to_db_branch();
	mysql_close(conn);

	return NULL;
}
void * recv_data(void *arg)
{
	int sockfd = -1;
	int recv_len=0;
	char cur_buff[1400]={0};
	struct sockaddr_in server_addr, client_addr;
	int sin_size = sizeof(struct sockaddr);
	pthread_t thread_id;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if (-1 == (sockfd = socket(AF_INET, SOCK_DGRAM, 0)))		
	{
		printf("%d\n", errno);
		exit(-1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(listenport);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_addr.sin_zero), 8);

	if (-1 == bind (sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)))
	{
		printf("%d\n", errno);
		goto err_out;
	}
printf("recv_data start\n");
	struct timeval tv;
	fd_set readfds;
	while(1)
	{
        tv.tv_sec=3;
        tv.tv_usec=10;
		FD_ZERO(&readfds);
        FD_SET(sockfd,&readfds);
        select(sockfd+1,&readfds,NULL,NULL,&tv);
        if(FD_ISSET(sockfd,&readfds))
        {
			recv_len = recvfrom(sockfd, &cur_buff, 1400, 0, (struct sockaddr *)&client_addr, (socklen_t *)&sin_size);
			if(-1 == recv_len)
			{
				printf("recv 2\n");
				printf("%d\n", errno);
				continue;
			}
			if(__sync_fetch_and_sub(&g_connection_state_db,0)){
				printf("recv 1\n");
					//接收成功
				pthread_create(&thread_id, &attr, data_ap_to_db, (void *)&cur_buff);
				pthread_create(&thread_id, &attr, data_db_to_ap, (void *)&client_addr);
			}

		}
	}

err_out:
	close(sockfd);
	exit(-1);
	return NULL;
}

void * connection_state_detection(void *arg)
{
	MYSQL *conn; 
	MYSQL_RES *res; 
	MYSQL_ROW row; 
	char sql[128];
	int ret;
    conn = mysql_init(NULL); 
	/* Connect to database */ 
	if (!mysql_real_connect(conn, db_service, 
		   user_name, password, mysql_db, db_port, NULL, 0)) { 
	   fprintf(stderr, "%s\n", mysql_error(conn)); 
	   exit(1); 
	}
		//唯一索引统计个数distinct(Ap_id)
	snprintf(sql, 100, "select distinct(Ap_Mac) from %s;",mysql_communication_table);
	while(1){
		sleep(1);
		ret = mysql_query(conn, sql);
		//if (ret == CR_SERVER_LOST || ret == CR_SERVER_GONE_ERROR) { 
		if(ret == 0)
		{
			printf("%s\n",sql);
			//Commands out of sync; you can't run this command now
			//process_result_set(res); /* client function */
			//mysql_free_result(res);
			//continue;
		}else{
			printf("2 %s\n",sql);
			printf("%s\n", mysql_error(conn)); 
			__sync_lock_test_and_set(&g_connection_state_db,0);
			continue;
		}
		if((res = mysql_store_result(conn) )==NULL){
			continue;
		}
		__sync_lock_test_and_set(&g_connection_state_db,1);
		//标记更新下发数据数组
		int i,j;
		int  num_fields;
		MYSQL_FIELD  *fields;
		row = mysql_num_rows(res);
		fields = mysql_fetch_fields(res);               //获取查询结果中，各个字段的名字
		num_fields = mysql_num_fields(res);             //获取查询结果中，字段的个数
		for( i = 0; i < num_fields; i++)
		{
			if(strcmp("Ap_Mac", fields[i].name) == 0){
				printf("location Ap_Mac %d\n", i);
				break;
			}
			//printf("数据长度%u \t  数据内容%s",lengths[i],row[i]?row[i]:"NULL");
		}

		while((row = mysql_fetch_row(res)))            //遍历查询结果中的各行记录
		{
			  unsigned long *lengths = NULL;
			  lengths = mysql_fetch_lengths(res);     //获取每一个记录行中，每一个字段的长度，在lengths数组中。
			  for( j = 0; i < num_fields; i++)
			  {
					printf("%u\t%s",lengths[i],row[i]?row[i]:"NULL");
			  }
			__sync_lock_test_and_set(&update_k[atoi(row[i])], 1);
		}
		mysql_free_result(res);
	}
	mysql_close(conn);
	return 0;
}
int main(int argc, char** argv)
{

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, recv_data, NULL);

	pthread_t thread_id_1;
	pthread_create(&thread_id_1, NULL, connection_state_detection, NULL);

	pthread_join(thread_id, NULL);
	pthread_join(thread_id_1, NULL);
	
	
	return 0;
}




