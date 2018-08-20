#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <linux/sockios.h>  


typedef struct cpu_info         //定义一个cpu occupy的结构体
{
unsigned int user; //定义一个无符号的int类型的user
unsigned int nice; //定义一个无符号的int类型的nice
unsigned int system;//定义一个无符号的int类型的system
unsigned int idle; //定义一个无符号的int类型的idle
unsigned int iowait;
unsigned int irp;
unsigned int softirp;
unsigned int stealstolen;
unsigned int guest;
}cpu_occupt;


typedef struct ap_local_info         //定义一个mem occupy的结构体
{
unsigned long mem_total; 
unsigned long mem_free;
double cpu_idle_rate;
}ap_local_info_t;


get_memoccupy (ap_local_info_t *mem) //对无类型get函数含有一个形参结构体类弄的指针O
{
    FILE *fd;          
    int n;             
    char buff[256];   
    ap_local_info_t *m;
	char tmp[30];
    m=mem;
	
    system("cat /proc/meminfo | sed -n '1,2p' > /workPorjectCode/t/cpuinfo.txt"); 
	
    fd = fopen ("/workPorjectCode/t/cpuinfo.txt", "r"); 
 
    fgets (buff, sizeof(buff), fd); 
    sscanf (buff, "%s %u", tmp, &m->mem_total); 
    fgets (buff, sizeof(buff), fd); 
    sscanf (buff, "%s %u", tmp, &m->mem_free); 
    printf ("%u %u\n", tmp, m->mem_total, m->mem_free); 

    fclose(fd);     //关闭文件fd
}

double cal_cpuoccupy (cpu_occupt *o, cpu_occupt *n) 
{   
    unsigned long one, two;    
    unsigned long idle, sd;
	unsigned long totalcputime;
    double cpu_use = 0;   

    one = (unsigned long) (o->user + o->nice + o->system +o->idle+o->iowait+o->irp+ o->softirp+ o->stealstolen+ o->guest);//第一次(用户+优先级+系统+空闲)的时间再赋给od
    two = (unsigned long) (n->user + n->nice + n->system +n->idle+n->iowait+n->irp+ n->softirp+ n->stealstolen+ n->guest);//第二次(用户+优先级+系统+空闲)的时间再赋给od
      
    idle = (unsigned long) (n->idle - o->idle);    //用户第一次和第二次的时间之差再赋给id
    //sd = (unsigned long) (n->system - o->system);//系统第一次和第二次的时间之差再赋给sd
	
	totalcputime = two-one;
    if(totalcputime != 0)
    cpu_use = (idle)*100.0/totalcputime; //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used
    else cpu_use = 0.0;
    return cpu_use;
}

get_cpuoccupy (cpu_occupt *cpust) //对无类型get函数含有一个形参结构体类弄的指针O
{   
    FILE *fd;         
    int n;            
    char buff[256]; 
	char tmp[30];
    cpu_occupt *cpu_occupt;
    cpu_occupt=cpust;
             
    system("cat /proc/stat | sed -n '1,1p' > /workPorjectCode/t/cpuinfo.txt;cat /proc/stat | sed -n '1,1p' >> /workPorjectCode/t/cpuinfo.txt"); 
	
    fd = fopen ("/workPorjectCode/t/cpuinfo.txt", "r"); 
    fgets (buff, sizeof(buff), fd);
    
    sscanf (buff, "%s %u %u %u %u %u %u %u %u %u",
		tmp, &cpu_occupt->user, &cpu_occupt->nice,&cpu_occupt->system, &cpu_occupt->idle, &cpu_occupt->iowait,
		&cpu_occupt->irp, &cpu_occupt->softirp, &cpu_occupt->stealstolen, &cpu_occupt->guest );

    printf ("%s %u %u %u %u %u %u %u %u %u\n",
		tmp, cpu_occupt->user, cpu_occupt->nice,cpu_occupt->system, cpu_occupt->idle, cpu_occupt->iowait,
		cpu_occupt->irp, cpu_occupt->softirp, cpu_occupt->stealstolen, cpu_occupt->guest );

    fclose(fd);     
}

int main()
{
    cpu_occupt cpu_stat1;
    cpu_occupt cpu_stat2;
    ap_local_info_t ap_info;
    double cpu_idle;
    
		//获取内存
		get_memoccupy ((ap_local_info_t *)&ap_info);
    
	
		//第一次获取cpu使用情况
		get_cpuoccupy((cpu_occupt *)&cpu_stat1);
		sleep(1);
		//第二次获取cpu使用情况
		get_cpuoccupy((cpu_occupt *)&cpu_stat2);
		
		//计算cpu使用率
		cpu_idle = cal_cpuoccupy ((cpu_occupt *)&cpu_stat1, (cpu_occupt *)&cpu_stat2);
		printf("%f\n",cpu_idle);

    return 0;
}