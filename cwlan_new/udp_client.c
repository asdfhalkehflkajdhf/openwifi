
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <errno.h>

#define 	SERVER_IP 		"192.168.200.242"
#define     CLIENT_IP       "127.0.0.2"
#define 	SERVER_PORT 	22222
#define     CLIENT_PORT     22223
#define		BUFFLENTH		4096
#define     BUFFLENER       50
#define     ERROUT          -1
#define     SUCCESS         0


int main(int argc, char** argv)
{
	if(argc !=3){
		printf("argc error  addr port \n");
		return 0;
	}
	int sockfd = -1;
	struct sockaddr_in client_addr, server_addr;
	char *msg = "Hello! World! This is UDP Client infomation!", buf[BUFFLENTH];
    int sin_size = sizeof(struct sockaddr_in);

	/* 获取套接字描述符 */
	if (-1 == (sockfd = socket(AF_INET, SOCK_DGRAM, 0)))
	{
		printf("%d\n", errno);
		return ERROUT;
	}
    printf("Client:sockfd = %d\n",sockfd);

    /* 设置服务器端基本配置信息 */
	server_addr.sin_family =AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	bzero(&(server_addr.sin_zero), 8);
    int i;
	for(i=0; i<1;i++){
    /* 向服务器端发送数据信息 */
   // printf("Client(send before):sendmMsg = %s\nlen = %d\n", msg, strlen(msg));
    sendto(sockfd, msg, BUFFLENER, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) ;
	
		struct timeval tv;
		fd_set readfds;
		FD_ZERO(&readfds);
        FD_SET(sockfd,&readfds);
        tv.tv_sec=3;
        tv.tv_usec=10;
        select(sockfd+1,&readfds,NULL,NULL,&tv);
        if(FD_ISSET(sockfd,&readfds))
        {
			if(-1 == recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &sin_size))
			{
				printf("%d\n", errno);
				goto err_out;
			}
		}
	}
    

err_out:
    /* 关闭套接字描述符 */
	close(sockfd);
    printf("Client:sockfd closed!\n");
	return SUCCESS;
}





