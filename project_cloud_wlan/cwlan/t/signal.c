#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
static void int_hander(int s)
{
	printf("Catch a signal sigint\n");
	exit(0);
}
int
main(void)
{
	int i;
	struct sigaction act, oact;
	act. sa_handler = int_hander;
	sigemptyset(&act. sa_mask); //清空此信号集
	act. sa_flags = 0;
	//sigaction(SIGQUIT , &act, &oact);
	signal(SIGINT , int_hander);
	while(1)
	{
		for(i=0; i<5; i++)
		{
			write(1, ".", 1);
			sleep(1);
		}
			write(1, "\n", 1);
	}
	//sigaction(SIGINT, &oact, NULL); //恢复成原始状态
return 0;
}