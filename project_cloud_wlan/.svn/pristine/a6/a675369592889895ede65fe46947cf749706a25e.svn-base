//关于htonl htons 的问题: 什么时候用l, 什么时候用s.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<time.h>
#include <arpa/inet.h>
int main()
{
	char date_gmt[35] = {0};

	struct tm *ptr;
	time_t timep;
	time(&timep);
	ptr = localtime(&timep);
	strftime(date_gmt,35,"%a, %d %h %Y %T GMT",ptr);
	
	
	void *aaa;
	
	
	
	printf("%x  %x\n",0x80,ntohl(0xa0521241));
	printf("%x  %x\n",0x80,ntohl(0xa0521474));
	printf("443 %x\n",htons(0xa0521241));
	printf("161 %x\n",htons(0xa0521474));

	return 0 ;
}