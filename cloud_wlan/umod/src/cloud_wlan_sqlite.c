#include <stdlib.h>  
#include <stdio.h>  
#include <unistd.h>  
#include <linux/netlink.h>  
#include <sys/socket.h>  
#include <strings.h>  
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

//extern s32 h_errno;


#include "cloud_wlan_types.h"
#include "cloud_wlan_nl_out_pub.h"
#include "cloud_wlan_sqlite.h"
#include "cloud_wlan_ap_com.h"



s8 g_cw_sql[G_CW_SQL_BUF_LEN]={0};  
sqlite3   *g_cw_db=NULL;  
s8	   *zErrMsg = 0;  

s8 *sqlite3_get_value(sqlite3_res *res, u32 nrow, s8 *fields, u32 *ret)
{
	u32 i;
	if(nrow <= 0 || nrow >res->nrow)
	{
		*ret = CWLAN_FAIL;
		return NULL;
	}

	for(i=0; i<res->ncolumn; i++)
	{
		if(strcmp(res->sqlite3_data[i], fields))
		{
			continue;
		}
		*ret = CWLAN_OK;
		return res->sqlite3_data[ res->ncolumn * nrow + i];
	}

	*ret = CWLAN_FAIL;
	return NULL;
}
/*
参数:	行号从１开始，fields为数据库中的字段名,
返回值:CWLAN_OK, CWLAN_FAIL
*/
u32 sqlite3_get_u32(sqlite3_res *res, u32 nrow, s8 *fields, u32 *dst)
{
	u32 ret;
	*dst = (u32)atoi(sqlite3_get_value(res, nrow, fields, &ret));
	return ret;
}

u32 sqlite3_get_s32(sqlite3_res *res, u32 nrow, s8 *fields, s32 *dst)
{
	u32 ret;
	*dst = (s32)atoi(sqlite3_get_value(res, nrow, fields, &ret));
	return ret;
}
u32 sqlite3_get_u8(sqlite3_res *res, u32 nrow, s8 *fields, u8 **dst)
{
	u32 ret;
	*dst = (u8 *)sqlite3_get_value(res, nrow, fields, &ret);
	return ret;
}
u32 sqlite3_get_s8(sqlite3_res *res, u32 nrow, s8 *fields, s8 **dst)
{
	u32 ret;
	*dst = (s8 *)sqlite3_get_value(res, nrow, fields, &ret);
	
	return ret;
}
u32 sqlite3_exec_get_res(sqlite3 *db, s8 *sql, sqlite3_res *res)
{
	return sqlite3_get_table( db , sql ,&res->sqlite3_data, &res->nrow , &res->ncolumn , &zErrMsg );  
}

u32 sqlite3_exec_free_res(sqlite3_res *res)
{
	sqlite3_free_table( res->sqlite3_data);
	return CWLAN_OK;
}
u32 sqlite3_exec_unres(sqlite3 *db, s8 *sql)
{
	return sqlite3_exec( db , sql , 0 , 0, &zErrMsg );
}
#if 0
u32 sqlite3_binary_write1(sqlite3 *db, s8 *sql, s8 *data, u32 data_len)
{
	sqlite3_stmt * stat = NULL;
	u32 ret;
	
	sqlite3_prepare( db, g_cw_sql, -1, &stat, 0 );
	if(stat == NULL)
	{
		return CWLAN_FAIL;
	}
	
	sqlite3_bind_blob( stat, 1, data, data_len, NULL );
	ret = sqlite3_step( stat );
	sqlite3_finalize( stat ); //把刚才分配的内容析构掉

	return CWLAN_OK;
}
u32 sqlite3_binary_read(sqlite3 *db, s8 *sql, s8 **data, u32 column)
{
	sqlite3_stmt * stat;
	u32 data_len = 0;

	sqlite3_prepare( db, sql, -1, &stat, 0 );
	sqlite3_step( stat );
	
	*data = sqlite3_column_blob( stat, column );
	data_len = sqlite3_column_bytes( stat, column );

	sqlite3_finalize( stat ); //把刚才分配的内容析构掉
	return data_len;
}
#endif

u32 sqlite3_update_date(sqlite3 *db, s8 *table, s8 *str, ...)
{
    s8 where[G_CW_SQL_BUF_LEN] = {0};
	s8 sql[G_CW_SQL_BUF_LEN];
	u32 ret;
	
	va_list args;
	va_start(args, str);
	vsnprintf(where, G_CW_SQL_BUF_LEN, str, args);
	va_end(args);

	snprintf(sql, G_CW_SQL_BUF_LEN, "UPDATE %s SET %s ;",table, where);
	ret = sqlite3_exec_unres(db, sql);
	if(ret != CWLAN_OK)
	{
		printf("sqlite3_exec[%s]: %s\n",sql, sqlite3_errmsg(db));  
		return CWLAN_FAIL;
	}
	return CWLAN_OK;
}

u32 sqlite3_insert_date(sqlite3 *db, s8 *table, s8 *field, s8 *str, ...)
{
    s8 where[G_CW_SQL_BUF_LEN] = {0};
	s8 sql[G_CW_SQL_BUF_LEN];
	u32 ret;
	va_list args;
	va_start(args, str);
	vsnprintf(where, G_CW_SQL_BUF_LEN, str, args);
	va_end(args);

	snprintf(sql, G_CW_SQL_BUF_LEN, "INSERT INTO %s(%s)VALUES(%s);",table,field, where);
	ret = sqlite3_exec_unres(db, sql);
	if(ret != CWLAN_OK)
	{
		printf("sqlite3_exec[%s]: %s\n",sql, sqlite3_errmsg(db));  
		return CWLAN_FAIL;
	}
	return CWLAN_OK;
}

u32 sqlite3_delete_date(sqlite3 *db, s8 *table, s8 *str, ...)
{
    s8 where[G_CW_SQL_BUF_LEN] = {0};
	s8 sql[G_CW_SQL_BUF_LEN];
	u32 ret;
	va_list args;
	va_start(args, str);
	vsnprintf(where, G_CW_SQL_BUF_LEN, str, args);
	va_end(args);

	snprintf(sql, G_CW_SQL_BUF_LEN, "DELETE FROM %s WHERE %s;",table,where);
	ret = sqlite3_exec_unres(db, sql);
	if(ret != CWLAN_OK)
	{
		printf("sqlite3_exec[%s]: %s\n",sql, sqlite3_errmsg(db));  
		return CWLAN_FAIL;
	}
	return CWLAN_OK;
}

u32 sqlite3_table_show(sqlite3 *db, s8 *table, s8 *field, s8 *str, ...)
{
	sqlite3_res *res;
	u32 ret;
	u32 i,j;
    s8 where[G_CW_SQL_BUF_LEN] = {0};
	s8 sql[G_CW_SQL_BUF_LEN];
	va_list args;
	va_start(args, str);
	vsnprintf(where, G_CW_SQL_BUF_LEN, str, args);
	va_end(args);

	snprintf(sql, G_CW_SQL_BUF_LEN, "SELECT %s FROM %s WHERE %s;",field, table,where);
	ret = sqlite3_exec_get_res(db, sql, res);
	if(ret != CWLAN_OK)
	{
		printf("sqlite3_exec[%s]: %s\n",g_cw_sql, sqlite3_errmsg(g_cw_db));  
		return CWLAN_FAIL;
	}
	for(i=0; i<=res->nrow; i++)
	{
		for(j=0; j<res->ncolumn; j++)
		{
			printf("%s|", res->sqlite3_data[res->ncolumn * i + j]);
		}
		printf("\n");
	}
	sqlite3_exec_free_res(res);

	return CWLAN_OK;
}

