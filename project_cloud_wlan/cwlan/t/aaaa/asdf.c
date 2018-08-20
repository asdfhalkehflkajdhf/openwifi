#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

#define IPIPE_CONFIG_FILE "config/ipipe.ini"
#define CLIENTS_FILE_PATH "config/clients"

int main()
{
	time_t ipipe_ini = 0;
	time_t clients = 0;

	struct stat filestat;
	while(1)
	{

		stat(IPIPE_CONFIG_FILE, &filestat);
		if(ipipe_ini < filestat.st_ctime)
		{
			ipipe_ini = filestat.st_ctime;
			printf("call ipipe config mod\n");
		}
		stat(CLIENTS_FILE_PATH, &filestat);
		if(clients < filestat.st_ctime)
		{
			clients = filestat.st_ctime;
			printf("call clients config mod\n");
		}

		sleep(3);
	};
	return 0;
}

