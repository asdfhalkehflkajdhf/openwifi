
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <errno.h>


#define 	BACKLOG			5
#define 	SERVER_IP 		"127.0.0.1"
#define 	SERVER_PORT 	22222
#define		BUFFLENTH		4096
#define     BUFFLENER       50
#define     ERROUT          -1
#define     SUCCESS         0


int main(int argc, char** argv)
{
	int sockfd = -1;
	struct sockaddr_in server_addr, client_addr;
    int sin_size = sizeof(struct sockaddr_in);
	char *msg = "Hello! World! This is UDP Server infomation!", buf[BUFFLENTH];
    if(argc !=3){
		printf("argc error  addr port \n");
		return 0;
	}
	/* 获取套接字描述符 */	
	if (-1 == (sockfd = socket(AF_INET, SOCK_DGRAM, 0)))        
	{
		printf("%d\n", errno);
		return ERROUT;
	}
    printf("Server:sockfd = %d\n",sockfd);

    /* 设置服务器端信息 */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	bzero(&(server_addr.sin_zero), 8);

	/* 指定一个套接字使用的端口 */
	if (-1 == bind (sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)))
	{
		printf("%d\n", errno);
		goto err_out;
	}
	printf("Server:Bind succeed!\n");
int i=0;
	while(1)
	{
		buf[0]='\0';
			/* 接收客户机端请数据信息 */
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
		printf("%s\n",buf);
	}
    
    
    
err_out:
    /* 关闭套接字sockfd描述符 */
	close(sockfd);
    printf("Server:sockfd closed!\n");
	return SUCCESS;
}




