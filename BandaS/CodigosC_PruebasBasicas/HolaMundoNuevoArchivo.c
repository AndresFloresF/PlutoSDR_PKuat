#include <stdio.h>
#include<stdlib.h>
#include <unistd.h>

int main (int argc,char **argv){
	int i = 0;
	while (i<10)
	{
        FILE* file_ptr = fopen("Prueba1.txt", "a");
        if (file_ptr == NULL)
        {
            printf("Error opening file\n");
            return 1;
        }
        else
        {
            fprintf(file_ptr, "Hola Mundo\t%d\n", i+1);
            fclose(file_ptr);
        }
		sleep(1);
		i++;
	}
	return 0;
}
