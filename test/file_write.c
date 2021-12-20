#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	printf("this is wrong!\n");
	if(argc!=3)
	{
		printf("usage: ./file_write filename contents\n");
	}
	FILE * f = fopen(argv[1], "w");
	fprintf(f, "%s", argv[2]);
	fclose(f);
	return 0;
}
