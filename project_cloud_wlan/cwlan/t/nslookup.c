
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
extern int h_errno;
/*
struct hostent {
    char *h_name;
    char **h_aliases;
    short h_addrtype;
    short h_length;
    char **h_addr_list;
    #define h_addr h_addr_list[0]
};
*/

#define SET_P_NULL(p) if((p) != NULL){*(p)='\0';}

char* get_Second_Level_Domain(char *dest)
{
	char * p = NULL;
	char tmpc, *tmpp;
	char * e = NULL;
	p = strchr(dest, ':');
	if(p == NULL)
	{
	/*url格式 1
	www.chinanews.com/gj/2014/04-16/6069354.shtml
	*/
		e = strchr(dest, '/');
		SET_P_NULL(e);
		printf("1 %s\n", dest);
		return dest;
	}
	
	if(strncmp(p, "://", 3) != 0)
	{
	/*url格式 2
	www.chinanews.com:8080/gj/2014/04-16/6069354.shtml
	*/
		SET_P_NULL(p);
		printf("2 %s\n", dest);
		return dest;
	}
	

	p = p+3;
	e = strchr(p, ':');
	if(e == NULL)
	{
	/*url格式 3
	http://www.chinanews.com/gj/2014/04-16/6069354.shtml
	*/
		e = strchr(p, 47);
		SET_P_NULL(e);
		printf("3 %s\n", p);

		return p;
	}
	else
	{
	/*url格式 4
	http://www.chinanews.com:8080/gj/2014/04-16/6069354.shtml
	*/
		SET_P_NULL(e);
		printf("4 %s\n", p);
		return p;
	}

}


int main(int argc, char *argv[])
{
        int               ret = -1;
        struct hostent    *host = NULL;   
        int               i = 0;
        char              ipstr[50];
		char *t;
		
		if(argc <2)
		{
			printf("Usage : <domain name>\n");
		}
		
		t = get_Second_Level_Domain(argv[1]);
		
        host = gethostbyname2(t, AF_INET);
        if (host == NULL)
        {
                herror("gethostbyname err");
                return -1;
        }

        printf("name = %s\n", host->h_name);

        for (i=0; host->h_aliases[i]!= NULL; i++)
        {
                printf("h_aliases[%d] = %s\n", i, host->h_aliases[i]);
        }

        
        for (i=0; host->h_addr_list[i]!= NULL; i++)
        {
                inet_ntop(AF_INET, host->h_addr_list[i], ipstr, sizeof(ipstr));
                printf("ipstr = %x\n",  *(int *)host->h_addr_list[i]);
        }

        

        return 0;
}