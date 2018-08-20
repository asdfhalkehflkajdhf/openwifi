#include <unistd.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>


#include "cloud_wlan_list.h"
#include "cloud_wlan_types.h"
#include "cloud_wlan_nl.h"
#include "cloud_wlan_ac_com.h"
#include "cloud_wlan_recv_info_branch.h"
#include "cloud_wlan_ap_info_list.h"


u32 server_port;
struct sockaddr_in server_addr;
s32 sockfd = -1;
void hexlog(unsigned char* str, int len)
{
	unsigned char* p = str;
	int out_len = len;
	char x[17];
	int nn;
	int j;
	if (len <= 0)
		return;
	printf( "----------------hex dump start-----------------\n");
	for (nn = 0; nn < out_len; nn++) {
		printf( "%02X ", p[nn]);
		if ((p[nn] & 0xff) < 0x20 || (p[nn] & 0xff) >= 0x7f)
			x[nn % 16] = '.';
		else
			x[nn % 16] = p[nn];
		if (nn % 16 == 15 || nn == out_len - 1) {
			if (nn == out_len - 1 && nn % 16 != 15)
				for (j = 0; j < 16 - (nn % 16); j++)
					printf( "	");
			printf( "\t\t %s\n", x);
			memset(x, 0, 17);
		}
	}
	printf( "\n----------------hex dump end-------------------\n");
}

u32 cw_sendto_info(struct sockaddr *client_addr, s8* data, u32 len)
{
	u32 sendsize=0;

	sendsize = 0;
	while( sendsize < len ) 				/* 向服务器发送数据信息 */
	{
		sendsize = sendsize + sendto(sockfd, data+sendsize, len - sendsize, 0, client_addr, sizeof(struct sockaddr));
		/* 每次发送后数据指针向后移动 */
	};
	return CWLAN_OK;
}

u32 cw_server_socket_init()
{
	if (-1 == (sockfd = socket(AF_INET, SOCK_DGRAM, 0)))        
	{
		printf("%d\n", errno);
		return CWLAN_FAIL;
	}
    printf("Server:sockfd = %d\n",sockfd);

    /* 设置服务器端信息 */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bzero(&(server_addr.sin_zero), 8);

	//本地地址和端口可重用，目录不需要也不能开放
	//setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&val, sizeof(s32)))

	/* 指定一个套接字使用的端口 */
	if (-1 == bind (sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)))
	{
		printf("%d\n", errno);
		return CWLAN_FAIL;
	}
	printf("Server:Bind succeed!\n");
	printf("\n========== INFOMATION ===========\n");
	printf("Server:sockfd = %d\n",sockfd);
	printf("Server:server IP   : %s\n",inet_ntoa(server_addr.sin_addr));
	printf("Server:server PORT : %d\n\n\n",ntohs(server_addr.sin_port));
	return CWLAN_OK;
}
s32 cw_ac_com_init(u32 argc, char ** argv)
{
	s32 ret=0;
	if(atoi(argv[1]) < 20000)
	{
		printf("ac com port [%d] need > 20000 \n",atoi(argv[1]));
		return CWLAN_FAIL;
	}
	server_port = atoi(argv[1]);
	
	if( argc >2 && atoi(argv[2]) != AP_OPEN_AUTH)
	{
		g_ap_auth_mode = AP_ADMIN_AUTH;
	}
	else if(argc <= 2)
	{
		g_ap_auth_mode = AP_OPEN_AUTH;
	}
	printf("ac com mode [%d]: 0 is open auth, not 0 is admin auth.\n",g_ap_auth_mode);
	/* 套接字初始化*/	
	ret = cw_server_socket_init();
	if(ret != CWLAN_OK)
	{
		printf("cw_server_socket_init fail\n");
		return CWLAN_FAIL;
	}
	ret = cw_ap_info_list_init();
	if(ret != CWLAN_OK)
	{
		printf("cw_ap_info_list_init fail\n");
		return CWLAN_FAIL;
	}
	ret = cw_recv_info_dispose_pthread_cfg_init();
	if(ret != CWLAN_OK)
	{
		printf("cw_recv_info_dispose_pthread_cfg_init fail\n");
		return CWLAN_FAIL;
	}
	return CWLAN_OK;
}
void cw_ac_com_exit(int sig)
{

    /* 关闭套接字sockfd描述符 */
	close(sockfd);
    printf("\nServer:sockfd closed!\n");

	cw_recv_info_dispose_pthread_cfg_exit();

	cw_ap_info_list_exit();

	exit(0);
}

void cw_ac_com_sig_init()
{
	signal(SIGINT, cw_ac_com_exit);
	signal(SIGQUIT, cw_ac_com_exit);
	signal(SIGILL, cw_ac_com_exit);//非常指令
	signal(SIGFPE, cw_ac_com_exit);//算述异常
	signal(SIGSEGV, cw_ac_com_exit);
	signal(SIGTERM, cw_ac_com_exit);//kill 不带参数
}

s32 main(int argc, char** argv)
{
	s32 ret=0;
    u32 sin_size = sizeof(struct sockaddr_in);
	s8 buf[MAX_PROTOCOL_LPAYLOAD] = {0};
	struct sockaddr client_addr;


	if(argc < 2 || argc >3)
	{
		printf("%s <server port> [ac auth mode] \n"
			"\tnote:ac auth mode: %d is open auth, not %d is admin auth.\n"
			"\t     servert prot need > 20000\n", 
			argv[0],AP_OPEN_AUTH, AP_OPEN_AUTH);
		return CWLAN_FAIL;
	}

	ret = cw_ac_com_init(argc, argv);
	if(ret != CWLAN_OK)
	{
		printf("cw ac com init fail\n");
		return CWLAN_FAIL;
	}
	
	cw_ac_com_sig_init();
	
	while(1)
	{
		/* 接收客户机端请数据信息 */
		ret = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &sin_size); 
		if(-1 == ret)
		{
			printf("%d\n", errno);
			//goto err_out;
			continue;
		}

		cw_recv_info_branch((dcma_udp_skb_info_t *)buf, ret, &client_addr);		

	}

	return CWLAN_OK;
}




