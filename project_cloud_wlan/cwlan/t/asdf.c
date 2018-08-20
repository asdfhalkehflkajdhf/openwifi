#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <linux/sockios.h>  
#define ETHTOOL_GLINK        0x0000000a /* Get link status (ethtool_value) */

typedef enum { IFSTATUS_UP, IFSTATUS_DOWN, IFSTATUS_ERR } interface_status_t;

typedef signed int u32;

/* for passing single values */
struct ethtool_value
{
    u32    cmd;
    u32    data;
};

interface_status_t interface_detect_beat_ethtool(int fd, char *iface)
{
    struct ifreq ifr;
    struct ethtool_value edata;
   
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name)-1);

    edata.cmd = ETHTOOL_GLINK;
    ifr.ifr_data = (caddr_t) &edata;

    if (ioctl(fd, SIOCETHTOOL, &ifr) == -1)
    {
        perror("ETHTOOL_GLINK failed ");
        return IFSTATUS_ERR;
    }

    return edata.data ? IFSTATUS_UP : IFSTATUS_DOWN;
}

int main(int argc, char *argv[])
{

        struct ifreq ifreq;
        int sock;

        if(argc!=2)
        {
                printf("Usage : ethname\n");
                return 1;
        }
        if((sock=socket(AF_INET,SOCK_STREAM,0))<0)
        {
                perror("socket");
                return 2;
        }
        strcpy(ifreq.ifr_name,argv[1]);
        if(ioctl(sock,SIOCGIFHWADDR,&ifreq)<0)
        {
                perror("ioctl");
                return 3;
        }
		printf("%d\n", interface_detect_beat_ethtool(sock, argv[1]));  
		printf("stats %d\n",ifreq.ifr_flags);
        printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                        (unsigned char)ifreq.ifr_hwaddr.sa_data[0],
                        (unsigned char)ifreq.ifr_hwaddr.sa_data[1],
                        (unsigned char)ifreq.ifr_hwaddr.sa_data[2],
                        (unsigned char)ifreq.ifr_hwaddr.sa_data[3],
                        (unsigned char)ifreq.ifr_hwaddr.sa_data[4],
                        (unsigned char)ifreq.ifr_hwaddr.sa_data[5]);
        return 0;
}