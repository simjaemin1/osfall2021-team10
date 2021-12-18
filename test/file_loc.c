#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>


int main(int argc, char *argv[])
{
	if(argc !=2)
	{
		printf("Usage: /test (filename)\n");
		return -1;
	}
	int error;
	error = syscall(399, argv[1], &loc);

}
