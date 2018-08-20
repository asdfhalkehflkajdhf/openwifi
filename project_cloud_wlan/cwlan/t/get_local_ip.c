#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <linux/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <stdlib.h>  
#include <linux/netlink.h>  
#include <strings.h>  
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdint.h>
#include <netdb.h>
#include <linux/sockios.h>

struct ethtool_value
{
    int    cmd;
    int    data;
};
#define ETHTOOL_GLINK        0x0000000a /* Get link status (ethtool_value) */

int get_interface_info(char *ifname, int *ip, char *mac)     
{   
    struct ifreq req;     
    struct sockaddr_in *host;    
    struct ethtool_value edata;
	int sockfd;
    if(-1 == (sockfd = socket(PF_INET, SOCK_STREAM, 0)))   
    {   
        perror( "socket" );     
        return -1;   
	}
  
    memset(&req, 0, sizeof(struct ifreq));     
    strcpy(req.ifr_ifrn.ifrn_name, ifname); 
    edata.cmd = ETHTOOL_GLINK;
again_get:
    edata.data = -1;
    req.ifr_data = (char *)&edata;
	ioctl(sockfd, SIOCETHTOOL, &req);
	if( edata.data != 1)
	{
		printf("interface %s is down again_get\n");  
		sleep(2);
		goto again_get;
	}
    ioctl(sockfd, SIOCGIFADDR, &req);
    host = (struct sockaddr_in*)&req.ifr_ifru.ifru_addr;    
	(*ip) = host->sin_addr.s_addr;
	if(mac != NULL)
	{
		ioctl(sockfd, SIOCGIFHWADDR, &req);
		memcpy(mac, req.ifr_ifru.ifru_hwaddr.sa_data, 6);
	}
    close(sockfd);     

    return 0;
} 

int main (int argc, char * argv[]) {

	int ip;
	get_interface_info(argv[1], &ip, NULL);
	printf("%x\n",ip);
    return 0;
}