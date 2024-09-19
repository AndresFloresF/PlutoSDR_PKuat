#include <stdio.h>
#include<stdlib.h>
#include <unistd.h>

int main (int argc,char **argv){
	int i = 0;
	while (i<10)
	{
		printf("Hola Mundo\t%d\n",i+1);
		printf("\n");
		sleep(1);
		i++;
	}
	return 0;
}
