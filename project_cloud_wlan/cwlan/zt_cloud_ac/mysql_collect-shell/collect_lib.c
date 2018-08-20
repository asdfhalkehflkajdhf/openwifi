#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>


#include <stdarg.h>
//#include <mysql/mysql.h>
#include "collect.h"

#define FIELDCOUNT 18

int set_bit(unsigned int kflag, int bit)
{
	bit = 1<<bit;
	kflag = kflag|bit;
	return kflag;
}
int get_bit(int kflag)
{
	int n;
	for(n=0; n<FIELDCOUNT; n++)
	{
		if((kflag>>n)%2==0)
		{
			return n;
		}
	}
	return n;
}

int split_string(char delimiter, char *string, int limit, char **string_array)
{   
    int count = 1;
    char *pchar;

    if(NULL == string || string[0] == '\0')
    {
        return 0;
    }
    
    if (0 == limit)
    {
        // È«²¿·Ö¸î
        limit = 99999;
    }
    
	string[strlen(string)-1]='\0';
    string_array[0] = string;
    
    pchar = string;
    while('\0' != *pchar && (int)count < limit)
    {
        if (delimiter == *pchar)
        {
			*pchar = '\0';
			string_array[count] = pchar+1;
            count++;
        }

        pchar++;
		
    }

	return count;
}

int set_offset(collect_file_info_t *collect_name)
{
	char cmd[50]={0};
	snprintf(cmd, 50, "echo '%d %d %d %d %d'>./offset", 
		(int)collect_name->sec, collect_name->yesr, collect_name->month, collect_name->day, collect_name->offset);
	system(cmd);
	return 0;
}
int get_offset(collect_file_info_t *collect_name)
{
	FILE *fd;
	char buff[40];
	int t_time;
	char path[50]={0};

	snprintf(path, 50, "./offset");

	fd = fopen (path, "r"); 
	if(fd == NULL)
	{
		memset((char *)collect_name, 0, sizeof(collect_file_info_t));
		DEBUG_U("init offset fail init all 0!\n");
		return 0;
	}
	fgets(buff, sizeof(buff), fd); 
	sscanf(buff, "%d %d %d %d %d",&t_time, &collect_name->yesr, &collect_name->month, &collect_name->day, &collect_name->offset); 
	fclose(fd);
	collect_name->sec = t_time;
	DEBUG_U("init offset ok!\n");
	return 0;
}

int collect_create_table(collect_file_info_t *current_name)
{
	char g_sql[COLLECT_MYSQL_LEN];

	char buf[COLLECT_MYSQL_LEN] = {0};
	char table_path[100] = {0};
	FILE *fd;

	snprintf(table_path, 100, "%s%s.txt",collect_table_dir, collect_table_name);
	
	fd = fopen (table_path, "r"); 
	if(fd == NULL)
	{
		printf("open fail %s\n",table_path);
		return -1;
	}

	fread(buf, 1, COLLECT_MYSQL_LEN, fd);


	snprintf(g_sql, COLLECT_MYSQL_LEN, buf, collect_table_name, current_name->yesr, current_name->month);
	SQLFILE_W("%s\n", g_sql);

	/* send SQL query  
	if (mysql_query(conn, g_sql))
	{ 
		printf("%s\n\nERROR: %s\n",g_sql ,mysql_error(conn)); 
		return -1;
	} 
	printf("init tables %s%4d%2d ok!\n", collect_table_name, current_name->yesr, current_name->month);*/

	fclose(fd);
	return 0;

}


