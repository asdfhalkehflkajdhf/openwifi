#ifndef COLLECT_H_
#define COLLECT_H_


extern int g_cw_debug;

/*这个日志不打印初始化日志信息*/
#define DEBUG_U(str, args...) \
       {\
            FILE *tfp;\
            if (g_cw_debug && (tfp = fopen("./collect.log", "a")) != NULL)\
            {\
					fprintf(tfp, str, ##args);\
                   	fclose(tfp);\
            }\
       }
#define SQLFILE_W(str, args...) \
       {\
            FILE *tfp;\
            if (g_cw_debug && (tfp = fopen("./collect.sql", "a")) != NULL)\
            {\
					fprintf(tfp, str, ##args);\
                   	fclose(tfp);\
            }\
       }

typedef struct record
{
	char action_request_time[20];
	char acct_status_type[16];
	char Framed_IP_Address[20];
	char Acct_Session_Id[30];
	unsigned int NAS_Port;
	char NAS_Port_Type[64];
	char Calling_Station_Id[64];
	unsigned int Acct_Delay_Time;
	unsigned int Acct_Session_Time;
	unsigned int Acct_Input_Octets;
	unsigned int Acct_Input_Gigawords;
	unsigned int Acct_Output_Octets;
	unsigned int Acct_Output_Gigawords;
	unsigned int Acct_Input_Packets;
	unsigned int Acct_Output_Packets;
	unsigned int Proxy_State;
	char User_Name[64];
	char NAS_IP_Address[20];
}record_t;

#define COLLECT_MYSQL_LEN 1024

extern char collect_service[100];
extern char collect_shell_name[100];
extern char collect_table_name[100];
extern char collect_act_dir[100];
extern char collect_table_dir[100];
extern char user_name[50];
extern char password[50];
extern char mysql_db[50];
extern int interval;


//MYSQL *conn; 
//MYSQL_RES *res; 
//MYSQL_ROW row; 

extern char sql[1024];

extern int set_bit(unsigned int kflag, int bit);

extern int get_bit(int kflag);


typedef struct collect
{
	time_t sec;
	int yesr;
	int month;
	int day;
	int offset;
}collect_file_info_t;


extern int split_string(char delimiter, char *string, int limit, char ** string_array);

extern int set_offset(collect_file_info_t *collect_name);

extern int get_offset(collect_file_info_t *collect_name);
extern int collect_create_table(collect_file_info_t *current_name);

#endif /* COLLECT_H_ */


