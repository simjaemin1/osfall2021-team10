#define _GNU_SOURCE
#include <unistd.h>
//#include <stdlib.h>
#include <sys/syscall.h>
//#include <sys/types.h>
#include <stdio.h>
#include <uapi/linux/sched/types.h>	// sched_param
#include <uapi/linux/sched.h>		// policies
int main(int argc, char *argv[])
{
	int pid = syscall(399);
	printf("pid is %d \n", pid);
	struct sched_param param = {0};	//
	int retval= syscall(156, pid, SCHED_WRR, &param);			//
	printf("retval: %d\n", retval);
	pid = syscall(399);
	printf("pid is %d \n", pid);
}
