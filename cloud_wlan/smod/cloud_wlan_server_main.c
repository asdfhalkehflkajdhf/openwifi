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
#include "create_dir.h"


int logsizeoflimit;
char logpath[128]={0};
int loglistenport;


char *conf_file=NULL;

#define	CONF_LOGSIZELIMIT "LOGSIZELIMIT="
#define	CONF_LOGPATH "LOGPATH="
#define	CONF_LOGLISTENPORT "LOGLISTENPORT="

int data_buff_queue = 0;

#define DATA_BUFF_SIZE 1024*1024*5
#define DATA_BUFF_NUM 10
#define DATA_BUFF_FULL 1
#define DATA_BUFF_NULL 0

typedef struct data_buff_
{
	int full_key;
	int top_index;
	char *data;
}data_buff_t;
data_buff_t g_buff[DATA_BUFF_NUM];


//pthread_attr_t attr;

static void process_enter(char *ptr)
{
    int len = strlen(ptr);
    int i;

    for (i=len-1; i>=0; i++) {
        if (ptr[i] == '\n') {
            ptr[i] = '\0';
            break;
        }
    }
}

int get_config(char *data, int data_len, char *keyword)
{
    FILE *fd;
    char *ptr;
    char buff[1024];

    fd = fopen(conf_file, "r");
    if (fd == NULL) {
        printf("ERROR: fopen %s\n", conf_file);
        return -1;
    }    

    while (fgets(buff, sizeof(buff), fd) != NULL) {
        if (buff[0] == '#')
            continue;

        if (strncmp(buff, keyword, strlen(keyword)) == 0) {
            ptr = strstr(buff, keyword);
            if (ptr == NULL) {
                printf("ERROR: get %s\n", keyword);
                fclose(fd);
                return -1;
            }
            strncpy(data, ptr+strlen(keyword), data_len);            
            process_enter(data);

            fclose(fd);
            return 0;
        }
    }

    fclose(fd);
    return -1;
}

int conf_init()
{
	char str[10];
    if (get_config(logpath, sizeof(logpath), CONF_LOGPATH) < 0) {
        printf("ERROR: get_config %s\n", CONF_LOGPATH);
        return -1;
    }

    //create logpath
    if (recursive_make_dir(logpath, DIR_MODE) != 0) {
        printf("ERROR: create %s\n", logpath);
        return -1;
    }

	if (get_config(str, sizeof(str), CONF_LOGSIZELIMIT) < 0) {
		 printf("ERROR: get_config %s\n", CONF_LOGSIZELIMIT);
		 return -1;
	}
	logsizeoflimit= atoi(str)*1024*1024;

	if (get_config(str, sizeof(str), CONF_LOGLISTENPORT) < 0) {
		printf("ERROR: get_config %s\n", CONF_LOGLISTENPORT);
		return -1;
	}
	loglistenport = atoi(str);

    return 0;
}

int run_init()
{
	int i;
	for(i=0; i<DATA_BUFF_NUM; i++){
		g_buff[i].full_key = DATA_BUFF_NULL;
		g_buff[i].top_index = 0;
		g_buff[i].data = malloc(DATA_BUFF_SIZE);
		if(g_buff[i].data == NULL){
			exit(-1);
		}
	}
	

	//pthread_attr_init(&attr);
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	return 0;
}

int get_log_file_path(char *filepath)
{
	time_t tNow =time(NULL);
	struct tm gmt = {0};	
 	localtime_r(&tNow, &gmt);
	
	snprintf( filepath, 512,
		"%s/%4d%02d%02d_%02d%02d%02d.txt",logpath,
		gmt.tm_year + 1900,gmt.tm_mon+1, gmt.tm_mday,
		gmt.tm_hour, gmt.tm_min, gmt.tm_sec);
	return 0;
}
data_buff_t * get_data_buff_full()
{
	int cur_index;
	for(cur_index = 0; cur_index<DATA_BUFF_NUM; cur_index++){
		if( DATA_BUFF_NULL != __sync_and_and_fetch(&g_buff[cur_index].full_key, DATA_BUFF_FULL)){
			return &g_buff[cur_index];
		}
	}
	return NULL;
}

data_buff_t * get_data_buff_null()
{
	int cur_index;
	while(1){
		cur_index = __sync_fetch_and_add(&data_buff_queue, 1)%DATA_BUFF_NUM;
		if( DATA_BUFF_NULL == __sync_and_and_fetch(&g_buff[cur_index].full_key, DATA_BUFF_FULL)){
			return &g_buff[cur_index];
		}
	}
	return NULL;
}

int create_log_fin_file( char *filepath)
{
	strcat(filepath, ".fin");
    FILE *fd = fopen(filepath, "w");
    if (fd == NULL)
    {
    	return -1;
    }
    fclose(fd);

    return 0;
}
void data_db_to_ap_branch()
{

}

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
		   user_name, password, mysql_db, 0, NULL, 0)) { 
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
	
	data_buff_t *temp=(data_buff_t *)arg;

	MYSQL *conn; 
	MYSQL_RES *res; 
	MYSQL_ROW row; 
	char sql[128];
    conn = mysql_init(NULL); 
	/* Connect to database */ 
	if (!mysql_real_connect(conn, db_service, 
		   user_name, password, mysql_db, 0, NULL, 0)) { 
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
	struct sockaddr_in server_addr, client_addr;
	int sin_size = sizeof(struct sockaddr);
	pthread_t thread_id;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	data_buff_t *cur_buff=NULL;
	if (-1 == (sockfd = socket(AF_INET, SOCK_DGRAM, 0)))		
	{
		printf("%d\n", errno);
		exit(-1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(loglistenport);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_addr.sin_zero), 8);

	if (-1 == bind (sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)))
	{
		printf("%d\n", errno);
		goto err_out;
	}

		struct timeval tv;
        tv.tv_sec=3;
        tv.tv_usec=10;
		fd_set readfds;
	while(1)
	{
		FD_ZERO(&readfds);
        FD_SET(sockfd,&readfds);
        select(sockfd+1,&readfds,NULL,NULL,&tv);
        if(FD_ISSET(sockfd,&readfds))
        {
			recv_len = recvfrom(sockfd, &cur_buff, 1400, 0, (struct sockaddr *)&client_addr, (socklen_t *)&sin_size);
			if(-1 == recv_len)
			{
				printf("%d\n", errno);
				continue;
			}
			if(__sync_fetch_and_sub(&g_connection_state_db,0)){
					//接收成功
				pthread_create(&thread_id_1, &attr, data_ap_to_db, (void *)&cur_buff);
				pthread_create(&thread_id_1, &attr, data_db_to_ap, (void *)&client_addr);
			}

		}
	}

err_out:
	close(sockfd);
	exit(-1);
	return NULL;
}

int g_connection_state_db = 0;
char update_k[100000]={0};
void * connection_state_detection(void *arg)
{
	MYSQL *conn; 
	MYSQL_RES *res; 
	MYSQL_ROW row; 
	char sql[128];
    conn = mysql_init(NULL); 
	/* Connect to database */ 
	if (!mysql_real_connect(conn, db_service, 
		   user_name, password, mysql_db, 0, NULL, 0)) { 
	   fprintf(stderr, "%s\n", mysql_error(conn)); 
	   return 0; 
	}
		//唯一索引统计个数distinct(Ap_id)
	snprintf(sql, 100, "select distinct(Ap_id) from %s;",mysql_communication_table);
	while(1){
		ret = mysql_query(conn, sql);
		if (ret == CR_SERVER_LOST || ret == CR_SERVER_GONE_ERROR) { 
			printf("%s\n",sql);
			printf("%s\n", mysql_error(conn)); 
			__sync_lock_test_and_set(&g_connection_state_db,0);
		continue;
		}else{
			printf("%s\n",sql);
			printf("%s\n", mysql_error(conn)); 
			continue;
		}
		if((res = mysql_store_result(conn) )==NULL{
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
			if(strcmp("Ap_id", fields[i].name) == 0){
				break;
			}
			//printf("数据长度%u \t  数据内容%s",lengths[i],row[i]?row[i]:"NULL");
		}

		while((row = mysql_fetch_row(res)))            //遍历查询结果中的各行记录
		{
			  unsigned long *lengths = NULL;
			  //lengths = mysql_fetch_lengths(res);     //获取每一个记录行中，每一个字段的长度，在lengths数组中。
			  //for( j = 0; i < num_fields; i++)
			  //{
				//	printf("数据长度%u \t  数据内容%s",lengths[i],row[i]?row[i]:"NULL");
			  //}
			  //update_k[atoi(row[i])]=1;
			__sync_lock_test_and_set(&update_k[atoi(row[i])], 1);

		}
		mysql_free_result(res);
	}
	mysql_close(conn);
	return 0;
}
int main(int argc, char** argv)
{
	int ret;
	if(argc != 2){
		printf("usage: error, not config file!\n");
	}
	conf_file = argv[1];
	ret = conf_init();
	if(ret != 0){
		printf("config file is error\n");
		return 0;
	}
	run_init();

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, recv_data, NULL);

	pthread_t thread_id_1;
	pthread_create(&thread_id_1, NULL, connection_state_detection, NULL);
	
	pthread_join(thread_id, NULL);
	pthread_join(thread_id_1, NULL);
	
	
	return 0;
}




