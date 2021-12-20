#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main (int argc, char**argv)
{
	FILE * f=fopen(argv[1], "w");
	fclose(f);
	return 0;
}
