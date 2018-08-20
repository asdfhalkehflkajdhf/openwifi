#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <getopt.h>
       #include <pthread.h>
       #include <string.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <unistd.h>
       #include <errno.h>
       #include <ctype.h>
#include <netinet/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include "netlib_list.h"
/*我们在使用signal和时钟相关的结构体之前，需要包含这两个头文件*/
#include <signal.h>
#include <sys/time.h>

#define HTTP_FRAGMENT_HASH_LEN 10000000

#define ERROR_OK 0
#define ERROR_FAIL -1

typedef struct reHttp_fragment_node
{
	struct list_head list;
	unsigned int srcIp;
	unsigned int destIp;
	unsigned short sPort;
	unsigned short dPort;
	int fragment_num;		//ÊÕµ½Á½¸ö°üÒª»ØÒ»¸öack
	time_t sec;
}reHttp_fragment_node_t;

typedef struct reHttp_fragment_hash
{
	struct list_head h_head;
}reHttp_fragment_hash_t;

reHttp_fragment_hash_t *g_reHttp_fragment_list;

typedef struct netlib_pkt_desc
{
	unsigned int saddr;
	unsigned int daddr;
	unsigned short source;
	unsigned short dest;
	
}netlib_pkt_desc_s;

static inline uint32_t rol32(uint32_t word, unsigned int shift)
{
	return (word << shift) | (word >> (32 - shift));
}



#define __jhash_final(a, b, c)			\
{						\
	c ^= b; c -= rol32(b, 14);		\
	a ^= c; a -= rol32(c, 11);		\
	b ^= a; b -= rol32(a, 25);		\
	c ^= b; c -= rol32(b, 16);		\
	a ^= c; a -= rol32(c, 4);		\
	b ^= a; b -= rol32(a, 14);		\
	c ^= b; c -= rol32(b, 24);		\
}
#define JHASH_INITVAL		0xdeadbeef

static inline uint32_t jhash_3words(uint32_t a, uint32_t b, uint32_t c,
                                    uint32_t initval)
{
  a += JHASH_INITVAL;
  b += JHASH_INITVAL;
  c += initval;

  __jhash_final(a, b, c);

  return c;
}

static unsigned int reHttp_get_hash(unsigned int a, unsigned int b, unsigned short c, unsigned short d, unsigned int mask)
{
  a += c;
  b += d;
  c = 0;
  a = a^b;
  b = a;
  return (jhash_3words(a,(a^c),(a|(b<<16)),0x5123598) & mask);
}

int reply_tcp_fragment_create(netlib_pkt_desc_s *pdesc)
{
	unsigned int hash_key;
	
	reHttp_fragment_hash_t *hash_head;
	reHttp_fragment_node_t *new_node;
	reHttp_fragment_node_t *node;
	reHttp_fragment_node_t *temp_node;

	hash_key = reHttp_get_hash(pdesc->saddr,pdesc->daddr,
		pdesc->source,pdesc->dest, HTTP_FRAGMENT_HASH_LEN-1);

	hash_head = g_reHttp_fragment_list + hash_key;

	
    list_for_each_entry_safe(node, temp_node, &hash_head->h_head, list)
	{
		if(node->destIp != pdesc->daddr || node->srcIp != pdesc->saddr ||
			node->dPort != pdesc->dest || node->sPort != pdesc->source)
		{
			continue;
		}
		printf(" [%x to %x] ct\t",node->srcIp,node->destIp);
		
		return ERROR_FAIL;
	
	}

	new_node = malloc(sizeof(reHttp_fragment_node_t));
	if(new_node != NULL)
	{
		new_node->destIp = pdesc->daddr;
		new_node->srcIp = pdesc->saddr;
		new_node->dPort = pdesc->dest;
		new_node->sPort = pdesc->source;
		new_node->fragment_num = 1;
		time(&new_node->sec);
		
		list_add(&new_node->list,&hash_head->h_head);

		//printf(" hash_key:%d %x to %x; %x to %x\n",hash_key,new_node->srcIp,new_node->destIp,new_node->sPort,new_node->dPort);
	}
	
	return ERROR_OK;
}
int reply_tcp_fragment_search_updata(netlib_pkt_desc_s *pdesc)
{
	unsigned int hash_key;
	unsigned int fragment_num;
	reHttp_fragment_hash_t *hash_head;
	reHttp_fragment_node_t *node;
	reHttp_fragment_node_t *temp_node;

	hash_key = reHttp_get_hash(pdesc->saddr,pdesc->daddr,
		pdesc->source,pdesc->dest, HTTP_FRAGMENT_HASH_LEN-1);

	hash_head = g_reHttp_fragment_list + hash_key;

    list_for_each_entry_safe(node, temp_node, &hash_head->h_head, list)
	{
		if(node->destIp != pdesc->daddr || node->srcIp != pdesc->saddr ||
			node->dPort != pdesc->dest || node->sPort != pdesc->source)
		{
			printf(" [%x to %x; %x to %x]",node->srcIp,node->destIp,node->sPort,node->dPort);
			continue;
		}
		
		node->fragment_num++;
		time(&node->sec);
		fragment_num = node->fragment_num;
		printf(" hash_key:%d %x to %x; %x to %x fragment_num:%d\n",hash_key,node->srcIp,node->destIp,node->sPort,node->dPort,fragment_num);
		if(fragment_num % 2 == 0)
		{
			return ERROR_OK;
		}	
	}
	printf(" hash_key:%d %x to %x; %x to %x find fail\n",hash_key,pdesc->saddr,pdesc->daddr,
		pdesc->source,pdesc->dest);	

	return ERROR_FAIL;

}
int reply_tcp_fragment_del(netlib_pkt_desc_s *pdesc)
{
	unsigned int hash_key;
	time_t tem_sec;
	reHttp_fragment_hash_t *hash_head;
	reHttp_fragment_node_t *node;
	reHttp_fragment_node_t *temp_node;

	hash_key = reHttp_get_hash(pdesc->saddr,pdesc->daddr,
		pdesc->source,pdesc->dest, HTTP_FRAGMENT_HASH_LEN-1);

	hash_head = g_reHttp_fragment_list + hash_key;
	
	list_for_each_entry_safe(node, temp_node, &hash_head->h_head, list)
	{
		if(node->destIp != pdesc->daddr || node->srcIp != pdesc->saddr ||
			node->dPort != pdesc->dest || node->sPort != pdesc->source)
		{
			continue;
		}
		printf(" hash_key:%d %x to %x; %x to %x del ok\n",hash_key,node->srcIp,node->destIp,node->sPort,node->dPort);
		list_del(&node->list);
		free(node);

		return ERROR_OK;
	
	}
	return ERROR_FAIL;

}
int reply_tcp_fragment_del_overtime(void)
{
	unsigned int hash_key;
	time_t tem_sec, start,end;
	reHttp_fragment_hash_t *hash_head;
	reHttp_fragment_node_t *node;
	reHttp_fragment_node_t *temp_node;

	time(&start);

	for(hash_key= 0 ;hash_key <HTTP_FRAGMENT_HASH_LEN; hash_key++)
	{
		hash_head = g_reHttp_fragment_list + hash_key;

		time(&tem_sec);
			
		list_for_each_entry_safe(node, temp_node, &hash_head->h_head, list)
		{
			if(tem_sec - node->sec <= 3)
			{
				continue;
			}
			//printf(" hash_key:%d %x to %x; %x to %x del ok\n",hash_key,node->srcIp,node->destIp,node->sPort,node->dPort);
			//list_del(&node->list);
			//free(node);
		}
	}
	
	time(&end);
	printf("sec time [%d]\n",end - start);

	return ERROR_OK;
}
long n = 0;//用来保存上一秒的时间和总共花去的时间
/*信号处理函数*/
void operate(void ) /* 定时事件代码 */
{ 
    printf("do operate! n=%d\n",n++);
	reply_tcp_fragment_del_overtime();
	
	
	netlib_pkt_desc_s pdesc;

	pdesc.saddr = 0x01010101;
	pdesc.daddr = 0x02020202;
	pdesc.source = 1234;
	pdesc.dest = 80;

	netlib_pkt_desc_s pdesc2;

	pdesc2.saddr = 0x03030303;
	pdesc2.daddr = 0x04040404;
	pdesc2.source = 1234;
	pdesc2.dest = 80;

	//reply_tcp_fragment_search_updata(&pdesc);
//reply_tcp_fragment_search_updata(&pdesc2);


}
void*  pthread_overtime(void* param)
{

while(1)
{

sleep(1);
operate();
}

		  return 0;
}
int main()
{
	int size;
	int i;
    size = sizeof(reHttp_fragment_hash_t)*HTTP_FRAGMENT_HASH_LEN;
    g_reHttp_fragment_list = (reHttp_fragment_hash_t*)malloc(size);
    if( NULL == g_reHttp_fragment_list )
    {
        //return ERROR_FAIL;
    }
    memset(g_reHttp_fragment_list, 0, size);
	for(i = 0 ; i< HTTP_FRAGMENT_HASH_LEN; i++)
	{
		list_head_init(&g_reHttp_fragment_list[i].h_head);
	}
	
	netlib_pkt_desc_s pdesc;

	pdesc.saddr = 0x01010101;
	pdesc.daddr = 0x02020202;
	pdesc.source = 1234;
	pdesc.dest = 80;
	reply_tcp_fragment_create(&pdesc);

	netlib_pkt_desc_s pdesc2;

	pdesc2.saddr = 0x03030303;
	pdesc2.daddr = 0x04040404;
	pdesc2.source = 1234;
	pdesc2.dest = 80;

	reply_tcp_fragment_create(&pdesc2);


reply_tcp_fragment_search_updata(&pdesc);
reply_tcp_fragment_search_updata(&pdesc2);

reply_tcp_fragment_del(&pdesc2);
reply_tcp_fragment_del(&pdesc);
reply_tcp_fragment_search_updata(&pdesc);
reply_tcp_fragment_search_updata(&pdesc2);
/*
*/
	time_t  start,end;
	time(&start);

	for(i = 0 ; i< HTTP_FRAGMENT_HASH_LEN; i++)
	{
		pdesc.saddr += 2;
		pdesc.daddr += 4;
		pdesc.source = 1234;
		pdesc.dest = 80;
		reply_tcp_fragment_create(&pdesc);
	}
		time(&end);

printf("init list ok:[%d]\n",end - start);

	pthread_t tidTracer;
	pthread_create(&tidTracer,NULL,pthread_overtime,NULL);


	while (1)
		sched_yield();

	return 0;
}

