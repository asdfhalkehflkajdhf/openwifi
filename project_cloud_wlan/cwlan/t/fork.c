#include <sys/resource.h>
#include <sys/time.h>



int set_coredump_size(void)
{
	int ret;
    struct rlimit rl;

	rl.rlim_cur = 2<<20;
    rl.rlim_max = 2<<20;
    ret = setrlimit(RLIMIT_CORE, &rl);
    if(ret < 0)
    {   
        perror("setrlimit()");
    }
    return ret;
}


int main()
{
	char * ptr=0;
	set_coredump_size();
	while(1)
	{
		*ptr=111;
	}
	return 0;
}