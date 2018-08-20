#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>

void *start_routine (void *arg)
{
    while (1) {
        sleep (1);
        printf ("start_routine\n");
    }
    return NULL;
}
char *ip_ntoa(char* ipbuf, int ina)
{
		   char *ucp = (char *)&ina;

		   sprintf(ipbuf, "%d.%d.%d.%d",
				   ucp[0] & 0xff,
				   ucp[1] & 0xff,
				   ucp[2] & 0xff,
				   ucp[3] & 0xff);
		   return ipbuf;
}
int main ()
{
	char buf[16]={0};
	printf("%s\n", ip_ntoa(buf, 0x01010101));
	
    return 0;
}