/*
gcc cse_msu_edu.c -lm

 ./a.out -1 1 1883
Enter a value for a: -1
Enter b value for b: 1
Enter n value for n: 1883
Enter dx value for dx: 0.166667
1: 0.014979 1.497907e-02

1880: 1.178975 3.963322e-06
1881: 1.178978 2.828024e-06
1882: 1.178980 1.695110e-06
1883: 1.178980 5.645858e-07
[root@localhost t]# 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


int main(int argc, char ** argv)
{
	int a,b,n;
	double dx,x,y;
	int i;
	double PI = 3.141592;
	double hh = 0, prohh = 0;
	double f;
	if(argc != 4 )
	{
		printf("Parameter error.Cse_msu a b n\n");
	}
	
	a = (int)strtol(argv[1], NULL, 10);
	b = (int)strtol(argv[2], NULL, 10);
	n = (int)strtol(argv[3], NULL, 10);
	
	if( a > b )
	{
		printf("Upper bound smaller than the lower limit!");
	}
	dx = (b - a)*1.0/n;
	
	printf("Enter a value for a: %d\nEnter b value for b: %d\nEnter n value for n: %d\nEnter dx value for dx: %f\n",a,b,n,dx);
	
	
	for(i = 1; i <= n; i++)
	{
		x = (a + (i - 0.5)*dx);
		if( x == 0 )
		{
			f = dx;
		}
		else
		{
			y = sin(PI * x)/(PI * x);
			f = y * dx;
		}
		prohh = hh;
		hh = hh + f;
		printf("%d: %f %e\n",i,hh,hh-prohh);
		if( i > 100000 || hh-prohh < 1E-10)
		{
			break;
		}
	}
	
	return 1;
	
}